#include <stdio.h>

#include "batteryControl_app.h"

#include "logger.h"

#include "batteryControl.h"
#include "canHelpers.h"

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
void batteryControl_appCyclicEntryPoint(void)
{
    // Initialize the CAN controller before sending Daly requests.
    if (!canHelpers_init(BATTERY_CAN_TX_GPIO, BATTERY_CAN_RX_GPIO)) {
        LOG_ERR("batteryControl", "CAN initialization failed");
    }
}

/**
 * Initialization function called once in the beginning
 */
void batteryControl_appInitAll(void)
{
    // Initialize the CAN controller before sending Daly requests.
    if (!canHelpers_init(BATTERY_CAN_TX_GPIO, BATTERY_CAN_RX_GPIO)) {
        LOG_ERR("batteryControl", "CAN initialization failed");
    }
}
