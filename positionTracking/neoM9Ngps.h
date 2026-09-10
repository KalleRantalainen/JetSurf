#ifndef NEO_M9N_GPS_H_
#define NEO_M9N_GPS_H_

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/uart.h"

// ESP32 UART pins connected to the GPS module:
// GPS module TX -> ESP32 GPIO16 (ESP32 RX)
// GPS module RX -> ESP32 GPIO17 (ESP32 TX)
#define GPS_UART UART_NUM_2
#define GPS_RX_GPIO GPIO_NUM_16
#define GPS_TX_GPIO GPIO_NUM_17
// Chinese seller says the default baudrate is 9600,
// but it seems to ne 38400 instead...
#define GPS_BAUD_RATE 38400

// GPS measurements will contain this data
typedef struct {
	bool hasFix;                 // Has determined its position, is ready
	double latitudeDegrees;      
	double longitudeDegrees;
	double speedMetersPerSecond;
	double courseDegrees;        // Course, 0...360deg, 0deg = north, 90deg = east, and so on.
	uint32_t fixTimestampMs;     // When the gps position (fix) was obtained
} gps_position_t;

/** Initialize the GPS UART and reset the latest position. */
void initGps(void);

/** Read and parse any complete NMEA sentences currently waiting in the UART. */
void readPosition(void);

/** Return the latest decoded GPS position and speed. */
bool neoM9Ngps_getPosition(gps_position_t *position);

#endif // NEO_M9N_GPS_H_