#ifndef BATTERYCONTROL_H_
#define BATTERYCONTROL_H_

#include <stdint.h>

#include "battery.h"

// Checks if any of the battery related warning/error
// flags should be triggered and triggers them.
void checkProtectionLimits(void);

/* These functions below use CAN to read the raw values from
   the battery. The final values for total voltage for example
   can be read using functions defined by battery.h */

// Reads battery SOC, voltage, and current from BMS
void readBatterySocVoltCur(Battery *battery);
// Reads the min and max voltages of the cells
void readBatteryMinMaxCellVolt(Battery *battery);
// Reads battery state, MOSFET state, cycle life, and remaining capacity
void readBatteryChargeStatus(Battery *battery);
// Reads the battery temperature sensors
void readBatteryTemps(Battery *battery);
// Reads the individual cell voltages
void readCellVoltages(Battery *battery);

#endif // BATTERYCONTROL_H_