#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stdint.h>

// All parameter signals defined here. Parameters are constant
// and will not change during runtime. Paraneters have
// have parameter_ prefix and are always constants

static const uint16_t parameter_highCurrentWarning = 50;      // Maximum current before warning
static const uint16_t parameter_maxCurrent = 55;              // Maximum current before reducing it drastically

static const uint16_t parameter_maxTemperature = 50;          // Maximum temperature in Celcius

static const float parameter_hardStopCellVoltageDiffMv = 150; // Maximum cell voltage (milliVolts) difference before stopping the motors

static const uint8_t minTotalVoltage = 40; // Volts, 2.5V per cell.
static const float minCellVoltage = 2.5;   // Minimum 2.5V for any singular cell

#endif // PARAMETERS_H