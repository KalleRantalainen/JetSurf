#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sdCardModule.h"

// Log queue, all messages added to this queue first
static QueueHandle_t s_logQueue = NULL;
static logger_output_t s_output = LOGGER_OUTPUT_TERMINAL;

/**
 * Create a timestamp from the current monotonic clock time.
 * This is captured when logLine() is called. Timestamp in seconds.
 * @param buffer the pointer to the buffer to which to write the timestamp to
 * @param bufferSize size of the timestamp buffer
 */
static void logger_formatTimestamp(char *buffer, size_t bufferSize)
{
    uint64_t nowS = (uint64_t)(esp_timer_get_time() / 1'000'000ULL);
    snprintf(buffer, bufferSize, "%llu s", (unsigned long long)nowS);
}

/**
 * Convert log level to its string presenstation
 * @param level log level as integer: 0, 1 or 2
 * @return level as string, 0 -> INFO, 1 -> WARNING, 2 -> ERROR
 */
static const char *logLevelToString(log_level_t level)
{
    switch (level) {
        case LOGGER_LEVEL_INFO:
            return "INFO";
        case LOGGER_LEVEL_WARNING:
            return "WARNING";
        case LOGGER_LEVEL_ERROR:
            return "ERROR";
        default:
            return "INFO";
    }
}

/**
 * Initialize the logger
 */
void logger_init(logger_output_t output)
{
    if (s_logQueue != NULL) {
        return;
    }

    // Create thread safe log queue for a maximum of 32 log lines
    s_logQueue = xQueueCreate(32, sizeof(log_entry_t));
    s_output = output;
    const bool sdCardRequested =
        s_output == LOGGER_OUTPUT_SD_CARD ||
        s_output == LOGGER_OUTPUT_SD_CARD_AND_TERMINAL;
    if (sdCardRequested && !sdCardModule_init()) {
        printf("SD card logging unavailable; using terminal logging.\n");
        s_output = LOGGER_OUTPUT_TERMINAL;
    }
}

/**
 * Destruct the logger
 */
void logger_deinit(void)
{
    if (s_logQueue == NULL) {
        return;
    }

    logger_drainQueue();
    vQueueDelete(s_logQueue);
    s_logQueue = NULL;
    sdCardModule_deinit();
}

/**
 * Add a single log line to the queue
 * @param level INFO, WARNING, ERROR
 * @param source string representation of the application, for example motorControl
 * @param fmt log message
 * @return true is success
 */
bool logLine(log_level_t level, const char *source, const char *fmt, ...)
{
    if (s_logQueue == NULL || source == NULL || fmt == NULL) {
        return false;
    }

    // Init log entry
    log_entry_t entry;
    memset(&entry, 0, sizeof(entry));

    // Capture the current time at the moment the log call was made.
    logger_formatTimestamp(entry.timestamp, sizeof(entry.timestamp));

    // Set the entry source
    entry.level = level;
    snprintf(entry.source, sizeof(entry.source), "%s", source);

    // Set the entry message.
    // vsnprintf safely truncates to the reserved 192-byte array
    // and returns a null-terminated string, so longer messages are cut off
    // without causing a buffer overflow or runtime error.
    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    // Put the log line to the queue and return the status
    // of that.
    return xQueueSend(s_logQueue, &entry, 0) == pdTRUE;
}

/**
 * Drain the log queue to the log file
 */
void logger_drainQueue(void)
{
    if (s_logQueue == NULL) {
        return;
    }

    log_entry_t entry;
    while (xQueueReceive(s_logQueue, &entry, 0) == pdTRUE) {
        char line[sizeof(entry.timestamp) + sizeof(entry.source) + sizeof(entry.message) + 32];
        int lineLength = snprintf(line, sizeof(line), "[%s] [%s] [%s] %s\n",
                                  entry.timestamp, logLevelToString(entry.level), entry.source, entry.message);
        if (lineLength <= 0) {
            continue;
        }

        if (s_output == LOGGER_OUTPUT_SD_CARD ||
            s_output == LOGGER_OUTPUT_SD_CARD_AND_TERMINAL) {
            if (!sdCardModule_write(line, (size_t)lineLength)) {
                printf("SD card write failed; dropping log line.\n");
            }
        }
        if (s_output == LOGGER_OUTPUT_TERMINAL ||
            s_output == LOGGER_OUTPUT_SD_CARD_AND_TERMINAL) {
            printf("%s", line);
        }
    }
}