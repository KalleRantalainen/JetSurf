#ifndef INPUT_SIGNALS_H
#define INPUT_SIGNALS_H

#include <stdint.h>

// All input signals of the whole application are declared here.
// Input is something that can only be read, for example,
// a current sensor value or a battery voltage.
// All signals declared here must have a prefix inputSignal_

// Throttle ranges from 0 to 1000
extern uint16_t inputSignal_throttle;

#endif