#include "batteryControl.h"
#include "canHelpers.h"

#include "logger.h"
#include "inputSignals.h"

#include "freertos/task.h"

/**
 * Sets the flags signals to true if battery currents
 * are getting or starting to get out of hand.
 */
static void checkCurrentLimits(void)
{
    // First check if the current is getting too high
    if (inputSignal_battery1_current >= parameter_highCurrentWarning) {
        outputSignal_battery1_currentIsGettingTooHigh = true;
        LOG_WARN("Battery 1 current is getting too high, limit: %.2f A, current: %.2f A",
                 parameter_highCurrentWarning,
                 inputSignal_battery1_current);
    } else {
        // The current was getting too high previously, now it has fallen below
        // the limit though, print that for later log inspections
        if (outputSignal_battery1_currentIsGettingTooHigh) {
            LOG_INFO("Current was getting too high, now below limit. Limit: %.2f A, current: %.2f",
                parameter_highCurrentWarning,
                inputSignal_battery1_current);
        }
        outputSignal_battery1_currentIsGettingTooHigh = false;
    }

    if (inputSignal_battery2_current >= parameter_highCurrentWarning) {
        outputSignal_battery2_currentIsGettingTooHigh = true;
        LOG_WARN("Battery 2 current is getting too high, limit: %.2f A, current: %.2f A",
                 parameter_highCurrentWarning,
                 inputSignal_battery2_current);
    } else {
        // The current was getting too high previously, now it has fallen below
        // the limit though, print that for later log inspections
        if (outputSignal_battery2_currentIsGettingTooHigh) {
            LOG_INFO("Battery 2 current was getting too high, now below limit. Limit: %.2f A, current: %.2f A",
                     parameter_highCurrentWarning,
                     inputSignal_battery2_current);
        }
        outputSignal_battery2_currentIsGettingTooHigh = false;
    }

    // Then check if the current is already too high
    if (inputSignal_battery1_current >= parameter_maxCurrent) {
        outputSignal_battery1_currentIsTooHigh = true;
        LOG_WARN("Battery 1 current is too high, limit: %.2f A, current: %.2f A",
                 parameter_maxCurrent,
                 inputSignal_battery1_current);
    } else {
        // The current was already too high previously, now it has fallen below
        // the limit though, print that for later log inspections
        if (outputSignal_battery1_currentIsTooHigh) {
            LOG_INFO("Battery 1 current was too high, now below limit. Limit: %.2f A, current: %.2f A",
                     parameter_maxCurrent,
                     inputSignal_battery1_current);
        }
        outputSignal_battery1_currentIsTooHigh = false;
    }

    if (inputSignal_battery2_current >= parameter_maxCurrent) {
        outputSignal_battery2_currentIsTooHigh = true;
        LOG_WARN("Battery 2 current is too high, limit: %.2f A, current: %.2f A",
                 parameter_maxCurrent,
                 inputSignal_battery2_current);
    } else {
        // The current was already too high previously, now it has fallen below
        // the limit though, print that for later log inspections
        if (outputSignal_battery2_currentIsTooHigh) {
            LOG_INFO("Battery 2 current was too high, now below limit. Limit: %.2f A, current: %.2f A",
                     parameter_maxCurrent,
                     inputSignal_battery2_current);
        }
        outputSignal_battery2_currentIsTooHigh = false;
    }
}

/**
 * Check the temperature readings of the highest temp sensors.
 * Set the flags if too high temp.
 */
static void checkTemperatureLimits(void)
{
    if (inputSignal_battery1_highestTemp >= parameter_maxTemperature) {
        outputSignal_battery1_temperatureTooHigh = true;
        LOG_WARN("Battery 1, sensor [%d] temperature is too high, limit: %.2f C, current: %.2f C",
                 inputSignal_battery1_highestTempSensor,
                 parameter_maxTemperature,
                 inputSignal_battery1_highestTemp);
    } else {
        outputSignal_battery1_temperatureTooHigh = false;
    }

    if (inputSignal_battery2_highestTemp >= parameter_maxTemperature) {
        outputSignal_battery2_temperatureTooHigh = true;
        LOG_WARN("Battery 2, sensor [%d] temperature is too high, limit: %.2f C, current: %.2f C",
                 inputSignal_battery2_highestTempSensor,
                 parameter_maxTemperature,
                 inputSignal_battery2_highestTemp);
    } else {
        outputSignal_battery2_temperatureTooHigh = false;
    }
}

/**
 * Check the cell voltage spread of each battery.
 * Set the flags if the difference becomes too high.
 */
void checkCellVoltageLimits(void)
{
    if (inputSignal_battery1_cellVoltageDiff >= parameter_hardStopCellVoltageDiffMv) {
        outputSignal_battery1_voltageDifferenceTooHigh = true;
        LOG_WARN("Battery 1 cell voltage diff is too high, limit: %.2f mV, current: %.2f mV",
                 parameter_hardStopCellVoltageDiffMv,
                 inputSignal_battery1_cellVoltageDiff);
    } else {
        // The cell voltage spread was previously too high, now it has fallen
        // below the limit though, print that for later log inspections.
        if (outputSignal_battery1_voltageDifferenceTooHigh) {
            LOG_INFO("Battery 1 cell voltage diff was too high, now below limit. Limit: %.2f mV, current: %.2f mV",
                     parameter_hardStopCellVoltageDiffMv,
                     inputSignal_battery1_cellVoltageDiff);
        }
        outputSignal_battery1_voltageDifferenceTooHigh = false;
    }

    if (inputSignal_battery2_cellVoltageDiff >= parameter_hardStopCellVoltageDiffMv) {
        outputSignal_battery2_voltageDifferenceTooHigh = true;
        LOG_WARN("Battery 2 cell voltage diff is too high, limit: %.2f mV, current: %.2f mV",
                 parameter_hardStopCellVoltageDiffMv,
                 inputSignal_battery2_cellVoltageDiff);
    } else {
        // The cell voltage spread was previously too high, now it has fallen
        // below the limit though, print that for later log inspections.
        if (outputSignal_battery2_voltageDifferenceTooHigh) {
            LOG_INFO("Battery 2 cell voltage diff was too high, now below limit. Limit: %.2f mV, current: %.2f mV",
                     parameter_hardStopCellVoltageDiffMv,
                     inputSignal_battery2_cellVoltageDiff);
        }
        outputSignal_battery2_voltageDifferenceTooHigh = false;
    }
}

/**
 * Checks if we are pulling too much current from the
 * batteries, or if the temps are getting too high etc.
 */
void checkProtectionLimits(void)
{
    checkCurrentLimits();
    checkTemperatureLimits();
    checkCellVoltageLimits();
}

// The Daly BMS id'S follow a pattern. When querying
// data from the BMS, the frame id is 0xaabbccdd where:
// - aa = priotity, example 0x18
// - bb = what data to query, example 0x90
// - cc = BMS id, for example 0x01
// - dd = querier id, for example 0x40

// Use 0x40 as the ESP32's id
static uint8_t ownId = 0x40;

/**
 * Helper function to sends a query frame and to get
 * a response frame.
 * @param battery the battery to query
 * @param dataQueryId the query id to put into the frame id, for example 0x90
 * @param queryName string to help in debugging, indicates what was queried
 * @param recvFrame data structure to which to append the received frame
 * @return true if the expected frame was received, false otherwise
 */
static bool requestBatteryFrame(Battery *battery, uint8_t dataQueryId,
                                 const char *queryName,
                                 canHelpers_frame_t *recvFrame)
{
    if (battery == NULL || recvFrame == NULL) {
        LOG_ERR("Cannot send %s query with a NULL argument",
                queryName);
        return false;
    }

    // Construct the query frame
    const uint8_t data[8] = {0};
    const uint32_t canId =
        ((uint32_t)battery->priority << 24) |
        ((uint32_t)dataQueryId << 16) |
        ((uint32_t)battery->bmsId << 8) |
        (uint32_t)ownId;
    // Construct the expected reposnse id.
    const uint32_t expectedResponseId =
        ((uint32_t)battery->priority << 24) |
        ((uint32_t)dataQueryId << 16) |
        ((uint32_t)ownId << 8) |
        (uint32_t)battery->bmsId;

    // LOG_INFO("batteryControl",
    //          "Sending %s query: id=0x%08lx, expected response=0x%08lx",
    //          queryName, (unsigned long)canId,
    //          (unsigned long)expectedResponseId);
    
    // Send the query frame, return false if sending fails
    if (!canHelpers_send(canId, data, sizeof(data), pdMS_TO_TICKS(100))) {
        LOG_ERR("Failed to send %s query: id=0x%08lx, BMS=0x%02x",
                queryName, (unsigned long)canId, battery->bmsId);
        return false;
    }

    // Timeout for getting the response frame
    const TickType_t timeoutTicks = pdMS_TO_TICKS(200);
    const TickType_t startTicks = xTaskGetTickCount();

    // Try to receive the reponse frame until time runs out.
    while ((xTaskGetTickCount() - startTicks) < timeoutTicks) {
        const TickType_t elapsedTicks = xTaskGetTickCount() - startTicks;
        const TickType_t remainingTicks = timeoutTicks - elapsedTicks;

        // Break the loop if the reciveiving fails
        if (!canHelpers_receive(recvFrame, remainingTicks)) {
            break;
        }

        // LOG_INFO("batteryControl",
        //          "Received %s frame: id=0x%08lx, extended=%s, dlc=%u, "
        //          "data=%02x %02x %02x %02x %02x %02x %02x %02x",
        //          queryName, (unsigned long)recvFrame->id,
        //          recvFrame->extended ? "yes" : "no", recvFrame->dataLength,
        //          recvFrame->data[0], recvFrame->data[1], recvFrame->data[2],
        //          recvFrame->data[3], recvFrame->data[4], recvFrame->data[5],
        //          recvFrame->data[6], recvFrame->data[7]);
        
        // Return true if the frame received had the expected id and data lenght
        if (recvFrame->extended && recvFrame->id == expectedResponseId &&
            recvFrame->dataLength == 8) {
            return true;
        }
    }

    // Return false if no valid response frame was received
    LOG_ERR("No valid %s response from BMS %u; expected id=0x%08lx",
            queryName, battery->bmsId, (unsigned long)expectedResponseId);
    return false;
}

/**
 * Reads SOC, total battery voltage and current.
 * @param battery battery to read
 */
void readBatterySocVoltCur(Battery *battery)
{
    canHelpers_frame_t recvFrame;
    if (!requestBatteryFrame(battery, 0x90, "SocVoltCur", &recvFrame)) {
        return;
    }

    // Voltage is in the first two bytes of the payload
    const uint16_t voltageRaw =
        ((uint16_t)recvFrame.data[0] << 8) | recvFrame.data[1];
    // Current is in the 5th and 6th byte of the payload
    const uint16_t currentRaw =
        ((uint16_t)recvFrame.data[4] << 8) | recvFrame.data[5];
    // SOC value is in the last two bytes of the payload
    const uint16_t socRaw =
        ((uint16_t)recvFrame.data[6] << 8) | recvFrame.data[7];

    // Write the values to the battery object
    battery->totalVoltage = voltageRaw;
    battery->current = currentRaw;
    battery->soc = socRaw;
}


void readBatteryMinMaxCellVolt(Battery *battery)
{
    canHelpers_frame_t recvFrame;
    if (!requestBatteryFrame(battery, 0x91, "minMaxCellVolt", &recvFrame)) {
        return;
    }
    // Max voltage in the first two bytes (millivolts)
    const uint16_t maxVoltage =
        ((uint16_t)recvFrame.data[0] << 8) | recvFrame.data[1];
    // The cell num that has the highest voltage
    const uint8_t maxVoltageCellNum = recvFrame.data[2];
    // Min voltage in the 4th and 5th byte
    const uint16_t minVoltage =
        ((uint16_t)recvFrame.data[3] << 8) | recvFrame.data[4];
    // The cell num with the lowest voltage
    const uint8_t minVoltageCellNum = recvFrame.data[5];

    // Store the information to the battery
    battery->maxCellVoltage = maxVoltage;
    battery->minCellVoltage = minVoltage;
    battery->maxVoltageCell = maxVoltageCellNum;
    battery->minVoltageCell = minVoltageCellNum;
}

/**
 * Read the battery charge status. This tells wether the 
 * battery is idle, charging or being drained. Also tells
 * the remaining capacity
 * @param battery the battery to read
 */
void readBatteryChargeStatus(Battery *battery)
{
    canHelpers_frame_t recvFrame;
    if (!requestBatteryFrame(battery, 0x93, "ChargeStatus", &recvFrame)) {
        return;
    }
    // Battery state stred in the first byte.
    // 00 = idle, 01 = charging, 02 = discharging
    battery->batteryState = recvFrame.data[0];
    // 00 = closed (allowed), 01 = open (blocked)
    battery->chargeMosfet = recvFrame.data[1];
    // 00 = closed (allowed), 01 = open (blocked)
    battery->dischargeMosfet = recvFrame.data[2];
    // BMS cycle life, total amount of charge/discharge cycles
    battery->cycleLife = recvFrame.data[3];
    // Remaining capacity in mAh
    battery->remainingCapacity =
        ((uint32_t)recvFrame.data[4] << 24) |
        ((uint32_t)recvFrame.data[5] << 16) |
        ((uint32_t)recvFrame.data[6] << 8) |
        (uint32_t)recvFrame.data[7];
}

/**
 * Read the battery's temperature sensors
 * @param battery the battery to read
 */
void readBatteryTemps(Battery *battery)
{
    canHelpers_frame_t recvFrame;
    if (!requestBatteryFrame(battery, 0x92, "Temperatures", &recvFrame)) {
        return;
    }

    // Max temperature in the first byte. This is latest value of the 
    // temperature sensor with the current highest reading.
    battery->maxTemperature = (int16_t)recvFrame.data[0];
    // The number of the temperature sensor with the highest reading.
    battery->maxTemperatureSensor = recvFrame.data[1];
    // Same for the minimum temperature.
    battery->minTemperature = (int16_t)recvFrame.data[2];
    battery->minTemperatureSensor = recvFrame.data[3];
}

void readCellVoltages(Battery *battery)
{
    // TODO: Implement. Probably not needed right now, will be
    // usefull if mobile app is implemented.
}
