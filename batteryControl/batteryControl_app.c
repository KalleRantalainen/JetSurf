#include <stdio.h>

#include "batteryControl_app.h"

#include "esp_timer.h"

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
    // CAN queries are somewhat slow. The query and response
    // take arounf 4us per bit. Since both contain about 126 bits,
    // one message takes about 500us or 0.5ms. This means getting
    // one response takes around 1ms. Addtionially some signals, like
    // temperature does not change meaningfully during one application
    // cycle, so it can be queried every 10 cycles for example
    static int cycle = 0;
    // Make sure the CAN communication is initialized
    if (!canHelpers_init(BATTERY_CAN_TX_GPIO, BATTERY_CAN_RX_GPIO)) {
        LOG_ERR("batteryControl", "CAN initialization failed");
    }

    if (canCalled < 1) {
        const int64_t startTimeUs = esp_timer_get_time();

        readBatterySocVoltCur(&blueBattery);
        readBatteryChargeStatus(&blueBattery);
        readBatteryTemps(&blueBattery);

        const int64_t elapsedTimeUs = esp_timer_get_time() - startTimeUs;
        LOG_INFO("batteryControl",
                 "Battery CAN reads completed in %lld.%03lld ms",
                 (long long)(elapsedTimeUs / 1000),
                 (long long)(elapsedTimeUs % 1000));

        canCalled++;
    }
}

/**
 * Initialization function called once in the beginning
 */
void batteryControl_appInitAll(void)
{
    // Initialize the CAN controller before sending Daly requests.
    if (!canHelpers_init(BATTERY_CAN_TX_GPIO, BATTERY_CAN_RX_GPIO)) {
        printf("CAN initialization failed\n");
    }

    // Initialize the batteries with proper ids and priorities.
    // The node id of the blue battery's BMS is confiugured to 2,
    // with daly configurator PC software, the gray is default 1.
    // All the message id values are hex, so they need to be hex
    // here as well.
    intializeBattery(&grayBattery, 0x01, 0x18);
    intializeBattery(&blueBattery, 0x02, 0x18);

    printf("Batteries initialized: gray BMS=0x%02x priority=0x%02x, "
           "blue BMS=0x%02x priority=0x%02x\n",
           grayBattery.bmsId, grayBattery.priority,
           blueBattery.bmsId, blueBattery.priority);
}
