#include <stdio.h>

#include "batteryControl_app.h"

#include "esp_timer.h"

#include "batteryControl.h"
#include "canHelpers.h"
#include "inputSignals.h"

#include "logger.h"

// One battery has a gray outershell, the other one
// has a blue shell.
static Battery grayBattery;  // Battery 1
static Battery blueBattery;  // Battery 2

/**
 * Read all local signals periodically
 */
static void readLocal(void)
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
        LOG_ERR("CAN initialization failed");
    }

    // Voltage and Current change fast and are 
    // essential to monitor closely to protect
    // all the components
    readBatterySocVoltCur(&blueBattery);
    readBatterySocVoltCur(&grayBattery);

    // Cell voltages also change fast, they are 
    // refreshed every iteration as well.
    readBatteryMinMaxCellVolt(&blueBattery);
    readBatteryMinMaxCellVolt(&grayBattery);

    // Signals that are updated every 10 cycles
    if (cycle % 10) {
        // Temperatures do not meaningfully change
        // every cycle, read them every 10 cycles.
        readBatteryTemps(&blueBattery);
        readBatteryTemps(&grayBattery);
        cycle = 0;
    }

    cycle++;
}

/**
 * Write all application's global signals periodically
 */
static void writeGlobal(void)
{
    inputSignal_battery1_current = getBatteryCurrent(&grayBattery);
    inputSignal_battery1_voltage = getBatteryVoltage(&grayBattery);
    inputSignal_battery1_soc = getBatterySoc(&grayBattery);
    inputSignal_battery1_highestTemp = getHighestTemp(&grayBattery);
    inputSignal_battery1_highestTempSensor = getHighestTempSensor(&grayBattery);
    inputSignal_battery1_cellVoltageDiff = getVoltageDiff(&grayBattery);
    LOG_SIGNAL("Battery1 current: %0.1f A\n", inputSignal_battery1_current);
    LOG_SIGNAL("Battery1 voltage: %0.1f V\n", inputSignal_battery1_voltage);
    LOG_SIGNAL("Battery1 SOC: %0.1f%%\n", inputSignal_battery1_soc);
    LOG_SIGNAL("Battery1 highest temp: %0.1f°C\n", inputSignal_battery1_highestTemp);
    LOG_SIGNAL("Battery1 highest temp sensor: %u\n", inputSignal_battery1_highestTempSensor);
    LOG_SIGNAL("Battery1 cell voltage diff: %0.1f mV\n", inputSignal_battery1_cellVoltageDiff);

    inputSignal_battery2_current = getBatteryCurrent(&blueBattery);
    inputSignal_battery2_voltage = getBatteryVoltage(&blueBattery);
    inputSignal_battery2_soc = getBatterySoc(&blueBattery);
    inputSignal_battery2_highestTemp = getHighestTemp(&blueBattery);
    inputSignal_battery2_highestTempSensor = getHighestTempSensor(&blueBattery);
    inputSignal_battery2_cellVoltageDiff = getVoltageDiff(&blueBattery);
    LOG_SIGNAL("Battery2 current: %0.1f A\n", inputSignal_battery2_current);
    LOG_SIGNAL("Battery2 voltage: %0.1f V\n", inputSignal_battery2_voltage);
    LOG_SIGNAL("Battery2 SOC: %0.1f\n%%", inputSignal_battery2_soc);
    LOG_SIGNAL("Battery2 highest temp: %0.1f\n°C", inputSignal_battery2_highestTemp);
    LOG_SIGNAL("Battery2 highest temp sensor: %u\n", inputSignal_battery2_highestTempSensor);
    LOG_SIGNAL("Battery2 cell voltage diff: %0.1f mv\n", inputSignal_battery2_cellVoltageDiff);
}

/**
 * Entry for the main interrupt loop. Reads and writes
 * all the application's signals
 */
void batteryControl_appCyclicEntryPoint(void)
{
    readLocal();
    writeGlobal();
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
