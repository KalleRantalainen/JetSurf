#include "battery.h"

/**
 * Sets the bmsId and priority for the battery
 * @param battery battery instance to which to set the id and prio for
 * @param bmsId the id of the battery's bms
 * @param prio priority that should be used with this battery's CAN messages,
 *             lower = higher priority.
*/
void intializeBattery(Battery* battery, uint8_t bmsId, uint8_t prio)
{
    battery->bmsId = bmsId;
    battery->priority = prio;
}

/**
 * Get battery current in Amps, from example 10.2A or -2.0A.
 * Positive when charging, negative when discharging
 * @param battery battery instance
 * @return current as float
 */
float getBatteryCurrent(Battery* battery)
{
    // Raw current value is uint16. Is is offset by
    // 30 000 and multiplied by 10. To get the
    // actual current, 30 000 has to be subracted
    // and then the result has to be divided by 10.
    float current = ((int32_t)battery->current - 30000) / 10.0f;
    return current;
}

/**
 * Get battery total voltage in Volts.
 * Example: 53.4V
 * @param battery battery instance
 * @return total battery voltage as float
 */
float getBatteryVoltage(Battery* battery)
{
    // Raw voltage is multiplied by 10, so it has to
    // be divided by 10 to get the voltage in volts
    float voltage = (float)battery->totalVoltage / 10.0f;
    return voltage;
}

/**
 * Get battery state of charge in percentages.
 * Example: 68%
 * @param battery battery instance
 * @return state of charge as float
 */
float getBatterySoc(Battery* battery)
{
    // SOC is multiplied by 10, divide by 10 to get the
    // proper percentage reading
    float soc = (float)battery->soc / 10.0f;
    return soc;
}

/**
 * Get the temperature of the sensor with the highest reading.
 * Example: 27C
 * @param battery battery instance
 * @return highest sensor temperature in degrees Celsius
 */
float getHighestTemp(Battery* battery)
{
    // Temp is offset by 40, subract 40 to get the actual Celcius reading.
    float temperature = (float)battery->maxTemperature - 40;
    return temperature;
}

/**
 * Get the ID of the sensor with the highest reading.
 * @param battery battery instance
 * @return sensor number as uint8_t
 */
uint8_t getHighestTempSensor(Battery* battery)
{
    // Can be returned as is.
    return battery->maxTemperatureSensor;
}

/**
 * Get the absolute voltage difference between the highest and lowest cell voltages.
 * @param battery battery instance
 * @return absolute cell voltage spread as float
 */
float getVoltageDiff(Battery* battery)
{
    // Should calculate the absolute value of the differnce
    // between the max and min voltage.
    int32_t diff = (int32_t)battery->maxCellVoltage - (int32_t)battery->minCellVoltage;
    if (diff < 0) {
        diff = -diff;
    }
    return (float)diff / 10.0f;
}