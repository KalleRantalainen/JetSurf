#ifndef BATTERYCONTROL_H_
#define BATTERYCONTROL_H_

#include <stdint.h>

float readBatterySOC(Battery *battery);
float readBatteryCurrent(Battery *battery);

#endif // BATTERYCONTROL_H_