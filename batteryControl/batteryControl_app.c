#include <stdio.h>

#include "batteryControl_app.h"

#include "logger.h"

#include "batteryControl.h"
#include "canHelpers.h"

// One battery has a gray outershell, the other one
// has a blue shell.
static Battery grayBattery;
static Battery blueBattery;

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
    // Make sure the CAN communication is initialized
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

    // Initialize the batteries with proper ids and priorities.
    // The node id of the blue battery's BMS is confiugured to 2,
    // with daly configurator PC software, the gray is default 1.
    // All the message id values are hex, so they need to be hex
    // here as well.
    intializeBattery(&grayBattery, 0x01, 0x18);
    intializeBattery(&blueBattery, 0x02, 0x19);
}
