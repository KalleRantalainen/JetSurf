#include "batteryControl.h"
#include "canHelpers.h"
#include "logger.h"

#include "freertos/task.h"

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
        LOG_ERR("batteryControl", "Cannot send %s query with a NULL argument",
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

    LOG_INFO("batteryControl",
             "Sending %s query: id=0x%08lx, expected response=0x%08lx",
             queryName, (unsigned long)canId,
             (unsigned long)expectedResponseId);
    
    // Send the query frame, return false if sending fails
    if (!canHelpers_send(canId, data, sizeof(data), pdMS_TO_TICKS(100))) {
        LOG_ERR("batteryControl",
                "Failed to send %s query: id=0x%08lx, BMS=0x%02x",
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

        LOG_INFO("batteryControl",
                 "Received %s frame: id=0x%08lx, extended=%s, dlc=%u, "
                 "data=%02x %02x %02x %02x %02x %02x %02x %02x",
                 queryName, (unsigned long)recvFrame->id,
                 recvFrame->extended ? "yes" : "no", recvFrame->dataLength,
                 recvFrame->data[0], recvFrame->data[1], recvFrame->data[2],
                 recvFrame->data[3], recvFrame->data[4], recvFrame->data[5],
                 recvFrame->data[6], recvFrame->data[7]);
        
        // Return true if the frame received had the expected id and data lenght
        if (recvFrame->extended && recvFrame->id == expectedResponseId &&
            recvFrame->dataLength == 8) {
            return true;
        }
    }

    // Return false if no valid response frame was received
    LOG_ERR("batteryControl",
            "No valid %s response from BMS %u; expected id=0x%08lx",
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

    // Voltage is multiplied by 10, so divide by 10 to get the
    // correct unit.
    const float totalVoltage = voltageRaw / 10.0f;
    // Current is offset by 30k and multiplied by 10. So subtract 30k
    // and divide by 10 to get the correct unit. Negative current 
    // means the battery is being drained, positive means charging
    const float current = ((int32_t)currentRaw - 30000) / 10.0f;
    // SOC is multiplied by 10. Divide by 10 to get the percentage value
    const float soc = socRaw / 10.0f;

    // Write the values to the battery object
    battery->totalVoltage = voltageRaw;
    battery->current = currentRaw;
    battery->soc = socRaw;

    LOG_INFO("batteryControl",
             "BMS %u: voltage=%.1f V, current=%.1f A, SOC=%.1f%%",
             battery->bmsId, totalVoltage, current, soc);
}


void readBatteryMinMaxCellVolt(Battery *battery)
{
    canHelpers_frame_t recvFrame;
    if (!requestBatteryFrame(battery, 0x91, "minMaxCellVolt", &recvFrame)) {
        return;
    }
    // Max voltage in the first two bytes
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

    LOG_INFO("batteryControl",
             "BMS %u: state=%u, charge MOSFET=%s, discharge MOSFET=%s, "
             "cycles=%u, remaining capacity=%lu mAh",
             battery->bmsId, battery->batteryState,
             battery->chargeMosfet == 0 ? "on" : "off",
             battery->dischargeMosfet == 0 ? "on" : "off",
             battery->cycleLife, (unsigned long)battery->remainingCapacity);
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
    // temperature sensor with the current highest reading. The temp is
    // offset by 40. So subract 40 to get Celsius.
    battery->maxTemperature = (int16_t)recvFrame.data[0] - 40;
    // The number of the temperature sensor with the highest reading.
    battery->maxTemperatureSensor = recvFrame.data[1];
    // Same for the minimum temperature.
    battery->minTemperature = (int16_t)recvFrame.data[2] - 40;
    battery->minTemperatureSensor = recvFrame.data[3];

    LOG_INFO("batteryControl",
             "BMS %u: max temperature=%d C (sensor %u), "
             "min temperature=%d C (sensor %u)",
             battery->bmsId, battery->maxTemperature,
             battery->maxTemperatureSensor, battery->minTemperature,
             battery->minTemperatureSensor);
}

void readCellVoltages(Battery *battery)
{
    // TODO: Implement.
}