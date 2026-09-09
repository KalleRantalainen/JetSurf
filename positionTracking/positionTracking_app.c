#include <stdio.h>

#include "positionTracking_app.h"

#include "inputSignals.h"
#include "logger.h"


/**
 * Read all application specific signals periodically
 */
static void readAll(void)
{
    // TODO: Read the position and speed from the GPS module
}

/**
 * Write all application speicific signals periodically
 */
static void writeAll(void)
{
    // No writes at the moment
}

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void motorControl_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called in the beginning
 */
void motorControl_appInitAll(void)
{
    // TODO initialize the GPS module
}