#ifndef BATTERY_H_
#define BATTERY_H_

#include <stdint.h>

// Both batteries are 16S LiFePO4 batteries
#define CELLCOUNT 16

// Battery contains a bms id and priority. These should be set to
// correct values during initialization and not changed after that.
typedef struct {
    // Fixed params
    uint8_t bmsId;    // 1 for the gray battery's bms, 2 for the blue
    uint8_t priority; // 18 by default

    // Variables
    uint16_t cellVoltages[CELLCOUNT]; // Individual cell voltages
    uint16_t totalVolatge;            // Total battery voltage
    int16_t current;                  // Negative when pulling current, positive when charging
    uint16_t soc;                     // Battery charge
    uint16_t temp1;                   // Temperature sensor 1 reading
    uint16_t temp2;                   // Temperature sensor 2 reading
    uint16_t maxCellVoltage;          // Maximum cell voltage
    uint16_t minCellVoltage;          // Minimum cell volatge
} Battery;

// Sets bmsId and CAN message priority for the battery
void intializeBattery(Battery* battery, uint8_t bmsId, uint8_t prio);

#endif // BATTERY_H_