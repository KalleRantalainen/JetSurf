#ifndef BLE_MASTER_H_
#define BLE_MASTER_H_

#include <stdbool.h>
#include <stdint.h>

// The future jetSurfRadioController must advertise this service and notify
// this characteristic with exactly one byte containing the throttle value.
#define BLE_MASTER_DEVICE_NAME "jetSurfRadioController"
// Set the throttle to 0 if the throttle has not been updated within
// the last 250ms. In this case the BLE connection has probably been lost
// and the surfboard has to be stopped.
#define BLE_MASTER_THROTTLE_TIMEOUT_MS 250U

void bleMaster_init(void);
void bleMaster_startScan(void);
uint8_t bleMaster_getThrottle(void);
bool bleMaster_isConnected(void);

#endif // BLE_MASTER_H_