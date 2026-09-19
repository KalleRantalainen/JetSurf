#include "outputSignals.h"

// All output signal definitions here

/* --- Motor throttle signals, same value for both motors */
uint8_t outputSignal_motor1_throttle;
uint8_t outputSignal_motor2_throttle;

/* --- Software level battery protection signals --- */
bool outputSignal_battery1_currentIsGettingTooHigh;
bool outputSignal_battery1_currentIsTooHigh;
bool outputSignal_battery1_temperatureTooHigh;
bool outputSignal_battery1_voltageDifferenceTooHigh;
bool outputSignal_battery1_totalVoltageTooLow;
bool outputSignal_battery1_cellVoltageTooLow;

bool outputSignal_battery2_currentIsGettingTooHigh;
bool outputSignal_battery2_currentIsTooHigh;
bool outputSignal_battery2_temperatureTooHigh;
bool outputSignal_battery2_voltageDifferenceTooHigh;
bool outputSignal_battery2_totalVoltageTooLow;
bool outputSignal_battery2_cellVoltageTooLow;

/* --- Control loop execution time signal in us --- */
int64_t outputSignal_controlLoopExecTimeUs;

/* --- LED signals for HMI interface to indicate the state of the control system --- */
bool outputSignal_gpsHasFix = false;
bool outputSignal_sdCardWritingOk = false;
bool outputSignal_bleThrottleOk = false;