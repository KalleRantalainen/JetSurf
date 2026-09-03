#include <stdio.h>

#include "batteryControl_app.h"

#include "logger.h"


/**
 * Read all application specific signals periodically
 */
static void readAll(void)
{
    // Read.
}

/**
 * Write all application speicific signals periodically
 */
static void writeAll(void)
{
    // Write
}

/**
 * Entry for the main interrupt loop. Reads and writes
 * all the application's signals
 */
void motorControl_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called once in the beginning
 */
void motorControl_appInitAll(void)
{
    // Init
}