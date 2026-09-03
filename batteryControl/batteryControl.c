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
 * Sends a query frame to get the soc, total voltage and
 * current from the BMS of the given battery
 */
void readBatterySocVoltCur(Battery *battery)
{
    if (battery == NULL) {
        LOG_ERR("batteryControl", "Cannot read SocVoltCur for a NULL battery");
        return;
    }

    // To read values from the bms, a read request has to be sent.
    // The payload of the request is 8 bytes of zeros.
    const uint8_t dlc = 8;
    const uint8_t data[8] = {0};
    const uint8_t dataQueryId = 0x90; // 90 stands for SOC, total voltage, and current

    // Construct the query frame id
    uint32_t canId =
        ((uint32_t)battery->priority << 24) |
        ((uint32_t)dataQueryId       << 16) |
        ((uint32_t)battery->bmsId << 8)  |
        ((uint32_t)ownId);

    // Send the message
    bool sendSuccess = canHelpers_send(
        canId,              // CAN ID
        data,               // payload
        dlc,                // payload length
        pdMS_TO_TICKS(100)  // timeout
    );

    // Reads the response message from the queue, if send was successfull
    if (sendSuccess) {
        // Init a structure for the reception
        canHelpers_frame_t recvFrame;
        const TickType_t timeoutTicks = pdMS_TO_TICKS(100);
        const TickType_t startTicks = xTaskGetTickCount();
        // Construct an expected response id. The own id and
        // bms id are expected to be flipped in the response.
        const uint32_t expectedResponseId =
            ((uint32_t)battery->priority << 24) |
            ((uint32_t)dataQueryId       << 16) |
            ((uint32_t)ownId             << 8)  |
            ((uint32_t)battery->bmsId);
        bool receiveSuccess = false;

        // Ignore unrelated frames until the expected BMS response arrives
        // or the overall timeout expires.
        while ((xTaskGetTickCount() - startTicks) < timeoutTicks) {
            TickType_t elapsedTicks = xTaskGetTickCount() - startTicks;
            TickType_t remainingTicks = timeoutTicks - elapsedTicks;

            if (!canHelpers_receive(&recvFrame, remainingTicks)) {
                break;
            }

            if (recvFrame.extended &&
                recvFrame.id == expectedResponseId &&
                recvFrame.dataLength == 8) {
                receiveSuccess = true;
                break;
            }
        }

        // Extract the response values as big-endian 16-bit fields.
        if (receiveSuccess) {
            // Total voltage in the first two bytes.
            const uint16_t voltageRaw =
                ((uint16_t)recvFrame.data[0] << 8) | recvFrame.data[1];
            // Current is in 5th and 6th bytes
            const uint16_t currentRaw =
                ((uint16_t)recvFrame.data[4] << 8) | recvFrame.data[5];
            // SOC value in the last two bytes
            const uint16_t socRaw =
                ((uint16_t)recvFrame.data[6] << 8) | recvFrame.data[7];

            // Raw voltage has a multiplier of 10, so divide by 10 to
            // get the actual voltage
            const float totalVoltage = voltageRaw / 10.0f;
            // Current has an offset of 30000, so raw value 30k
            // corresponds to 0 current. Anything less than 30k
            // is discharge current (negative) and anything more
            // than 30k is charge current (positive). Current is 
            // also multiplied by 10, so divide by 10 to get
            // the correct float value.
            const float current = ((int32_t)currentRaw - 30000) / 10.0f;
            // SOC value is multipled by ten, so divide by 10
            // to get the soc %.
            const float soc = socRaw / 10.0f;
            
            // Print the values for now. In reality, should update the
            // battery object and then the application signals should be
            // written from there.
            LOG_INFO("batteryControl",
                     "BMS %u: voltage=%.1f V, current=%.1f A, SOC=%.1f%%",
                     battery->bmsId, totalVoltage, current, soc);
        } else {
            LOG_ERR("batteryControl",
                    "No valid SocVoltCur response from BMS %u",
                    battery->bmsId);
        }
    } else {
        LOG_ERR("batteryControl", "Failed to send SocVoltCur query frame!");
    }

}


void readBatteryMinMaxCellVolt(Battery *battery)
{
    // TODO: Implement.
}

void readBatteryTemps(Battery *battery)
{
    // TODO: Implement.
}

void readCellVoltages(Battery *battery)
{
    // TODO: Implement.
}