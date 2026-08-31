#ifndef OUTPUT_SIGNALS_H
#define OUTPUT_SIGNALS_H

#include <stdint.h>

// All ouput signals of the whole application are declared here.
// Output signal is something that can be written, for example,
// a throttle value.
// All signals declared here must have a prefix outputSignal_

// Throttle ranges from 0 to 1000
extern uint16_t outputSignal_throttle;

#endif