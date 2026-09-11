#ifndef BATTERY_H_
#define BATTERY_H_

#include <stdint.h>

// Both batteries are 16S LiFePO4 batteries
#define CELLCOUNT 16

// Battery contains a bms id and priority. These should be set to
// correct values during initialization and not changed after that.
typedef struct {
    // Fixed params
    uint8_t bmsId;    // 0x1 for the gray battery's bms, 0x2 for the blue 
    uint8_t priority; // 0x18 by default

    // Variables
    uint16_t cellVoltages[CELLCOUNT]; // Individual cell voltages
    uint16_t totalVoltage;            // Total battery voltage
    uint16_t current;                 // Negative when pulling current, positive when charging, offset by 30 000
    uint16_t soc;                     // Battery charge
    uint16_t temp1;                   // Temperature sensor 1 reading
    uint16_t temp2;                   // Temperature sensor 2 reading
    int16_t maxTemperature;           // Maximum temperature in degrees Celsius
    int16_t minTemperature;           // Minimum temperature in degrees Celsius
    uint8_t maxTemperatureSensor;     // Sensor number reporting maxTemperature
    uint8_t minTemperatureSensor;     // Sensor number reporting minTemperature
    uint16_t maxCellVoltage;          // Maximum cell voltage
    uint16_t minCellVoltage;          // Minimum cell volatge
    uint8_t maxVoltageCell;           // Cell num with the highest voltage
    uint8_t minVoltageCell;           // Cell num with the smallest voltage
    uint8_t batteryState;             // 0 idle, 1 charging, 2 discharging
    uint8_t chargeMosfet;             // 0 closed/on, 1 open/off
    uint8_t dischargeMosfet;          // 0 closed/on, 1 open/off
    uint8_t cycleLife;                // Number of full charge/discharge cycles
    uint32_t remainingCapacity;       // Remaining capacity in mAh
} Battery;

// Sets bmsId and CAN message priority for the battery
void intializeBattery(Battery* battery, uint8_t bmsId, uint8_t prio);

/* -- Getters -- */
// Get battery current in Amps, from example 10.2A or -2.0A
float getBatteryCurrent(Battery* battery);
// Get battery current in Volts, fro example 53.4V
float getBatteryVoltage(Battery* battery);
// Get battery SOC in percentages, for example 68%
float getBatterySoc(Battery* battery);
// Get the temperature of the sensor with the highest reading
// in celcius, for example 27C
float getHighestTemp(Battery* battery);
// Get the id of the sensor with the highest reading, 1 or 2
uint8_t getHighestTempSensor(Battery* battery);
// Get the absolute voltage difference between cells that
// have the highest differece
float getVoltageDiff(Battery* battery);


#endif // BATTERY_H_