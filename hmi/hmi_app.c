#include <stdio.h>

#include "hmi_app.h"
#include "ledControl.h"

#include "logger.h"
#include "outputSignals.h"
#include "sdCardModule.h"

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void hmi_appCyclicEntryPoint(void)
{
    outputSignal_sdCardWritingOk = sdCardModule_isReady();
    cycleLeds();
}

/**
 * Initialization function called in the beginning
 */
void hmi_appInitAll(void)
{
    initializeLeds();
}