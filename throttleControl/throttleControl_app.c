#include "throttleControl_app.h"

#include "inputSignals.h"
#include "outputSignals.h"

/**
 * Read all application specific signals periodically
 */
static void readAll(void)
{
    // The Bluetooth application updates the input signal asynchronously.
}

/**
 * Write all application speicific signals periodically
 */
static void writeAll(void)
{
    // Write the throttle input signal as the throttle output
    outputSignal_throttle = inputSignal_throttle;
}

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void throttleControl_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called in the beginning
 */
void throttleControl_appInitAll(void)
{
    // Initialize the global throttle signals to 0.
    inputSignal_throttle = 0;
    outputSignal_throttle = 0;
}