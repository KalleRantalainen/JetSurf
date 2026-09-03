#ifndef BATTERYCONTROL_H_
#define BATTERYCONTROL_H_

#include <stdint.h>

#include "battery.h"

// Reads battery SOC, voltage, and current from BMS
void readBatterySocVoltCur(Battery *battery);
// Reads the min and max voltages of the cells
void readBatteryMinMaxCellVolt(Battery *battery);
// Reads the battery temperature sensors
void readBatteryTemps(Battery *battery);
// Reads the individual cell voltages
void readCellVoltages(Battery *battery);

#endif // BATTERYCONTROL_H_