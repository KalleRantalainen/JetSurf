#ifndef BATTERYCONTROL_H_
#define BATTERYCONTROL_H_

#include <stdint.h>

#include "battery.h"

// This provides an interface to read the values of 
// any battery. Functions here call the canHelpers to
// get and send CAN data from and to the bus and then
// interpret that data correctly to get the values

// float readBatterySOC(Battery *battery);
// float readBatteryCurrent(Battery *battery);
// and so on...

#endif // BATTERYCONTROL_H_