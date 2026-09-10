#include "outputSignals.h"

// All output signal definitions here

/* --- Motor throttle signals, same value for both motors */
uint16_t outputSignal_motor1_throttle;
uint16_t outputSignal_motor2_throttle;

/* --- Software level battery protection signals --- */
bool outputSignal_battery1_currentTooHigh;
bool outputSignal_battery1_temperatureTooHigh;
bool outputSignal_battery1_voltageDifferenceTooHigh;

bool outputSignal_battery2_currentTooHigh;
bool outputSignal_battery2_temperatureTooHigh;
bool outputSignal_battery2_voltageDifferenceTooHigh;
