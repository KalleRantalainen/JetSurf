#ifndef INPUT_SIGNALS_H
#define INPUT_SIGNALS_H

#include <stdint.h>

// All input signals of the whole application are declared here.
// Input is something that can only be read, for example,
// a current sensor value or a battery voltage.
// All signals declared here must have a prefix inputSignal_

/* --- Signals received over BLE --- */
extern uint8_t inputSignal_bleThrottle;  // BLE throttle ranges from 0 to 255, 0 = 0% throttle, 255 = 100% throttle

/* --- Position signals received from GPS module --- */
extern double inputSignal_velocityMetSec;   // Velocity in meters per second, 5cm/s accuracy?
extern double inputSignal_latitudeDeg;      // Latitude in degrees
extern double inputSignal_longitudeDeg;     // Longitude in degrees
extern double inputSignal_courseDeg;        // Heading in degrees, 0deg = North, 90deg = East, ...
extern uint32_t inputSignal_gpsTimestampMs; // Last time gps data was updates, M9N has update freq of 1Hz

/* --- Battery signals received over CAN --- */
extern float inputSignal_battery1_current;             // Current from/to battery 1
extern float inputSignal_battery1_voltage;             // Total voltage of battery 1
extern float inputSignal_battery1_soc;                 // SOC% of battery 1, for example 50% = 20Ah*0.5 = 10Ah
extern float inputSignal_battery1_highestTemp;         // Temperature in Celcius from the sensor with highest reading
extern uint8_t inputSignal_battery1_highestTempSensor; // The sensor that has the highest reading (sensor 1 or 2)
extern float inputSignal_battery1_cellVoltageDiff;     // Highest voltage difference between any two cells in volts

extern float inputSignal_battery2_current;             // Current from/to battery 2
extern float inputSignal_battery2_voltage;             // Total voltage of battery 2
extern float inputSignal_battery2_soc;                 // SOC% of battery 2, for example 50% = 20Ah*0.5 = 10Ah
extern float inputSignal_battery2_highestTemp;         // Temperature in Celcius from the sensor with highest reading
extern uint8_t inputSignal_battery2_highestTempSensor; // The sensor that has the highest reading (sensor 1 or 2)
extern float inputSignal_battery2_cellVoltageDiff;     // Highest voltage difference between any two cells in volts

#endif // INPUT_SIGNALS_H