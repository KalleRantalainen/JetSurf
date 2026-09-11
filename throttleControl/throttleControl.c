#include <stdlib.h>

#include "throttleControl.h"

#include "inputSignals.h"

// Maximum amount the throttle CAN be increased/decreased during
// one app cycle. This corresponds to about 20%.
static uint8_t maxThrottleChange = 50; 

/**
 * BLE throttle value is between 0 and 255. So the resturned value
 * should be something in that range. However, the throttle should
 * also not increase more than x % at a time. So we should smoothen
 * the throttle here if the value is increasing or decreasing too
 * quickly.
 */
uint8_t getValidThrottle(void)
{
    static uint8_t lastThrottleValue = 0;
    uint8_t newThrottleValue = (uint8_t)inputSignal_bleThrottle;
    int16_t diff = (int16_t)newThrottleValue - lastThrottleValue;

    uint8_t validThrottleValue;
    
    if (diff < 0) {
        // Throttle is decreasing
        if (abs(diff) > maxThrottleChange) {
            // Can only decrease the maximum change amount
            validThrottleValue = lastThrottleValue - maxThorttleChange;
        } else {
            validThrottleValue = newThrottleValue;
        }
    } else {
        // Throttle is increasing
        if (abs(diff) > maxThrottleChange) {
            // Can only increase the maximum change amount
            validThrottleValue = lastThrottleValue + maxThorttleChange;
        } else {
            validThrottleValue = newThrottleValue;
        }
    }
    
    // Update the lastThrottleValue to last valid value
    lastThrottleValue = validThrottleValue;
    return validThrottleValue;
}