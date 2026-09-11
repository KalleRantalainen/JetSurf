#ifndef THROTTLECONTROL_H_
#define THROTTLECONTROL_H_

#include <stdint.h>

// Validate the BLE throttle signal, either
// get it directly or get safe, modifed version
// of it. Will be between [0,255]
uint8_t getValidThrottle(void);

// Apply the battery protection flags to the valid throttle.
// When a battery warning or hard-stop signal is active, the
// throttle will be reduced progressively to keep the power
// draw in check. This value is what the rest of the system
// should use for the motor throttle.
uint8_t getProtectedThrottle(void);

#endif // THROTTLECONTROL_H_