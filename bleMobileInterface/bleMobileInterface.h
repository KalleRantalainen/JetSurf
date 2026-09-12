#ifndef BLE_MOBILE_INTERFACE_H_
#define BLE_MOBILE_INTERFACE_H_

#include <stdint.h>

// Called after the shared NimBLE host is synchronized so the remote
// controller client can begin scanning without creating a second host.
typedef void (*bleMobileInterface_scan_start_t)(void);

void bleMobileInterface_init(bleMobileInterface_scan_start_t scanStart);

#endif // BLE_MOBILE_INTERFACE_H_
