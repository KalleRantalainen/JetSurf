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