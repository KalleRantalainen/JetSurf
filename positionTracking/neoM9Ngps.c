#include "neoM9Ngps.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "neoM9Ngps";
static char s_sentence[128];
static size_t s_sentenceLength = 0;
static gps_position_t s_position;
static bool s_initialized = false;

/**
 * Convert an NMEA coordinate such as 6012.3456 into signed decimal degrees.
 * @param value NMEA degrees-and-minutes coordinate
 * @param hemisphere N/S for latitude or E/W for longitude
 * @return signed coordinate in decimal degrees
 */
static double nmeaCoordinateToDegrees(const char *value, char hemisphere)
{
    const double coordinate = strtod(value, NULL);
    const double degrees = (int)(coordinate / 100.0);
    double decimalDegrees = degrees + (coordinate - degrees * 100.0) / 60.0;
    if (hemisphere == 'S' || hemisphere == 'W') {
        decimalDegrees = -decimalDegrees;
    }
    return decimalDegrees;
}

/**
 * Verify the XOR checksum at the end of an NMEA sentence.
 * @param sentence complete sentence without the line ending
 * @return true when the sentence checksum is valid
 */
static bool nmeaChecksumIsValid(const char *sentence)
{
    if (sentence[0] != '$') {
        return false;
    }

    const char *checksumMarker = strchr(sentence, '*');
    if (checksumMarker == NULL || checksumMarker - sentence < 2) {
        return false;
    }

    unsigned char checksum = 0;
    for (const char *character = sentence + 1; character < checksumMarker; character++) {
        checksum ^= (unsigned char)*character;
    }

    unsigned int receivedChecksum = 0;
    if (sscanf(checksumMarker + 1, "%2x", &receivedChecksum) != 1) {
        return false;
    }
    return checksum == receivedChecksum;
}

/**
 * Decode a valid RMC sentence into the latest GPS position structure.
 * RMC supplies fix status, coordinates, speed over ground, and course.
 */
static void parseRmcSentence(char *sentence)
{
    if (!nmeaChecksumIsValid(sentence)) {
        return;
    }

    char *fields[13] = {0};
    char *field = strtok(sentence + 1, ",*");
    size_t fieldCount = 0;
    while (field != NULL && fieldCount < 13) {
        fields[fieldCount++] = field;
        field = strtok(NULL, ",*");
    }

    // RMC fields: type, time, status, latitude, N/S, longitude, E/W,
    // speed in knots, course, date, magnetic variation, E/W, mode.
    if (fieldCount < 10 ||
        (strcmp(fields[0], "GNRMC") != 0 && strcmp(fields[0], "GPRMC") != 0) ||
        fields[2][0] == '\0' || fields[3][0] == '\0' || fields[4][0] == '\0' ||
        fields[5][0] == '\0' || fields[6][0] == '\0' || fields[7][0] == '\0') {
        return;
    }

    const bool hasFix = fields[2][0] == 'A';
    s_position.hasFix = hasFix;
    if (!hasFix) {
        s_position.fixTimestampMs = (uint32_t)(esp_timer_get_time() / 1000ULL);
        return;
    }

    s_position.latitudeDegrees = nmeaCoordinateToDegrees(fields[3], fields[4][0]);
    s_position.longitudeDegrees = nmeaCoordinateToDegrees(fields[5], fields[6][0]);
    s_position.speedMetersPerSecond = strtod(fields[7], NULL) * 0.514444;
    s_position.courseDegrees = strtod(fields[8], NULL);
    s_position.fixTimestampMs = (uint32_t)(esp_timer_get_time() / 1000ULL);
}

/**
 * Initialize UART2 for the GPS module's default NMEA output.
 */
void initGps(void)
{
    const uart_config_t uartConfig = {
        .baud_rate = GPS_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if (uart_driver_install(GPS_UART, 2048, 0, 0, NULL, 0) != ESP_OK ||
        uart_param_config(GPS_UART, &uartConfig) != ESP_OK ||
        uart_set_pin(GPS_UART, GPS_TX_GPIO, GPS_RX_GPIO, UART_PIN_NO_CHANGE,
                     UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(TAG, "GPS UART initialization failed");
        return;
    }

    memset(&s_position, 0, sizeof(s_position));
    s_sentenceLength = 0;
    s_initialized = true;
    ESP_LOGI(TAG, "GPS UART initialized on RX=%d TX=%d at %d baud",
             GPS_RX_GPIO, GPS_TX_GPIO, GPS_BAUD_RATE);
}

/**
 * Read all available UART bytes and parse complete NMEA sentences.
 */
void readPosition(void)
{
    if (!s_initialized) {
        return;
    }

    uint8_t bytes[256];
    const int byteCount = uart_read_bytes(GPS_UART, bytes, sizeof(bytes), 0);
    for (int index = 0; index < byteCount; index++) {
        const char character = (char)bytes[index];
        if (character == '\n') {
            s_sentence[s_sentenceLength] = '\0';
            parseRmcSentence(s_sentence);
            s_sentenceLength = 0;
        } else if (character != '\r' && s_sentenceLength < sizeof(s_sentence) - 1) {
            s_sentence[s_sentenceLength++] = character;
        } else if (s_sentenceLength >= sizeof(s_sentence) - 1) {
            s_sentenceLength = 0;
        }
    }
}

/**
 * Poll the GPS UART for a newer speed value.
 * RMC contains position and speed together, so this delegates to the same parser.
 */
void readSpeed(void)
{
    readPosition();
}

/**
 * Copy the latest GPS data into the caller's structure.
 * @return true if a valid GPS fix has been received
 */
bool neoM9Ngps_getPosition(gps_position_t *position)
{
    if (position == NULL) {
        return false;
    }
    *position = s_position;
    return s_position.hasFix;
}