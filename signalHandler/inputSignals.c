#include "inputSignals.h"

// All input signal definitions here

/* --- Signals received over BLE --- */
uint8_t inputSignal_bleThrottle;

/* --- Position signals received from GPS module --- */
double inputSignal_velocityMetSec;
double inputSignal_latitudeDeg;
double inputSignal_longitudeDeg;
double inputSignal_courseDeg;
uint32_t inputSignal_gpsTimestampMs;

/* --- Battery signals received over CAN --- */
float inputSignal_battery1_current;
float inputSignal_battery1_voltage;
float inputSignal_battery1_soc;
float inputSignal_battery1_highestTemp;
uint8_t inputSignal_battery1_highestTempSensor;
float inputSignal_battery1_cellVoltageDiff;

float inputSignal_battery2_current;
float inputSignal_battery2_voltage;
float inputSignal_battery2_soc;
float inputSignal_battery2_highestTemp;
uint8_t inputSignal_battery2_highestTempSensor;
float inputSignal_battery2_cellVoltageDiff;