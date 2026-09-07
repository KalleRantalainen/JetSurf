#ifndef BLE_MASTER_H_
#define BLE_MASTER_H_

#include <stdbool.h>
#include <stdint.h>

// The future jetSurfRadioController must advertise this service and notify
// this characteristic with exactly one byte containing the throttle value.
#define BLE_MASTER_DEVICE_NAME "jetSurfRadioController"

void bleMaster_init(void);
uint8_t bleMaster_getThrottle(void);
bool bleMaster_isConnected(void);

#endif // BLE_MASTER_H_