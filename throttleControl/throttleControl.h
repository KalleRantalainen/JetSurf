#ifndef THROTTLECONTROL_H_
#define THROTTLECONTROL_H_

#include <stdint.h>

// Validate the BLE throttle signal, either
// get it directly or get safe, modifed version
// of it. Will be between [0,255]
uint8_t getValidThrottle(void);

#endif // THROTTLECONTROL_H_