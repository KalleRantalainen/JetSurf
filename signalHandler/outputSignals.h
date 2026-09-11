#ifndef OUTPUT_SIGNALS_H
#define OUTPUT_SIGNALS_H

#include <stdint.h>

// All ouput signals of the whole application are declared here.
// Output signal is something that can be written, for example,
// a throttle value.
// All signals declared here must have a prefix outputSignal_

/* --- Throttle signals --- */
extern uint8_t outputSignal_motor1_throttle; // Throttle ranges from 0 to 1000
extern uint8_t outputSignal_motor2_throttle; // The throttle values for both motors should be the same, no differential throttle

/* --- Software level battery protection signals --- */
extern bool outputSignal_battery1_currentIsGettingTooHigh;  // Actions to limit further current increase should be taken
extern bool outputSignal_battery1_currentIsTooHigh;         // Current drawn from battery 1 is too high, immediate action to lower the current should be taken
extern bool outputSignal_battery1_temperatureTooHigh;       // Temperature of either battery 1 temp sensor is too high
extern bool outputSignal_battery1_voltageDifferenceTooHigh; // Voltage difference between any too cells of battery 1 is too high

extern bool outputSignal_battery2_currentIsGettingTooHigh;  // Actions to limit further current increase should be taken
extern bool outputSignal_battery2_currentIsTooHigh;         // Current drawn from battery 2 is too high, lower it NOW
extern bool outputSignal_battery2_temperatureTooHigh;       // Temperature of either battery 2 temp sensor is too high
extern bool outputSignal_battery2_voltageDifferenceTooHigh; // Voltage difference between any too cells of battery 2 is too high

#endif // OUTPUT_SIGNALS_H