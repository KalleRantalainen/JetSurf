#include <stdio.h>

#include "logger_app.h"

#include "logger.h"

/**
 * Read all signals periodically.
 */
static void readAll(void)
{
    // Logger has no read signals at the moment.
}

/**
 * Drain the queue and emit queued log lines to the configured sink.
 */
static void writeAll(void)
{
    logger_drainQueue();
}

/**
 * Logger loop calls this periodically.
 */
void logger_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called when the logger thread starts.
 */
void logger_appInitAll(void)
{
    logger_init();
}