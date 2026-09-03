#ifndef BATTERY_H_
#define BATTERY_H_

#include <stdint.h>
// TODO: Declares a battery struct. This will be needed as
// there will be more than one battery.

// Should this file write the values gotten 

typedef struct {
    // What could be needed here? Would each battery contain
    // SOC, voltage, current, temperatures, ind. cell voltages, etc..?
    // Maybe not? Maybe we just read what ever we need and write the signals
    // to the signalHandler? Maybe this Battery struct is just a handle to
    // a specific battery?
    // Maybe we need information about to which CAN
    uint8_t id; // 1 for the gray battery's bms, 2 for the blue

} Battery;

#endif // BATTERY_H_