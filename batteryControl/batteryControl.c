#include "batteryControl.h"
#include "canHelpers.h"
#include "logger.h"

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
    // To read values from the bms, a read request has to be sent.
    // The payload of the request is 8 bytes of zeros.
    uint8_t dlc = 8;
    uint8_t data[dlc] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t dataQueryId = 90; // 90 stands for SOC, total voltage, and current

    // Construct the query frame id
    uint32_t canId = 
        ((uint32_t)battery->prio  << 24) |
        ((uint32_t)0x90           << 16) |
        ((uint32_t)battery->bmsId << 8)  |
        ((uint32_t)ownId);

    // Send the message
    bool_t sendSuccess = canHelpers_send(
        canId,              // CAN ID
        data,               // payload
        dlc,                // payload length
        pdMS_TO_TICKS(100)  // timeout
    );

    // Reads the response message from the queue, if send was successfull
    if (sendSuccess) {
        // Init a structure for the reception
        canHelpers_frame_t recvFrame;
        // Receive a frame
        bool_t receiveSuccess = canHelpers_receive(&recvFrame, pdMS_TO_TICKS(100))
        
        // Extract data from the recived frame, if recived something
        // if (receiveSuccess) {
        //     typedef struct {
        //         uint32_t id;
        //         bool extended;
        //         uint8_t dataLength;
        //         uint8_t data[TWAI_FRAME_MAX_LEN];
        //     } canHelpers_frame_t;

        // TODO: Check that the recived frame id is what it is expected to be.
        // Here is how that should work:
        // So the example message sent would be
        // 18900140 8 00 00 00 00 00 00 00 00
        // And the response to this would be
        // 18904001 8 02 15 00 00 75 30 03 D8

        // Then read the data from the payload. First two bytes is 10*totalVoltage
        // so in the example voltage would be 0x0215 = 533 = 53.3V
        // 3rd and 4th bytes are reserved, 00 and 00.
        // 5th and 6th byte is the current with 30 000 offset and 
        // multiplied by ten. In the example:
        // 0x7530 = 30000, then 30000 - 30000 = 0 * 0.1 = 0.0A
        // negative means pulling current, positive means charging current.
        // Last two is SOC, multiplied by 10. So in the example:
        // 0x03D8 = 984 = 98.4%
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