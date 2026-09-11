#include <stdlib.h>

#include "throttleControl.h"

#include "inputSignals.h"
#include "outputSignals.h"

// Maximum amount the throttle CAN be increased/decreased during
// one app cycle. This corresponds to about 20%.
static uint8_t maxThrottleChange = 50;

static uint8_t applyReduction(uint8_t throttle, uint8_t percent)
{
    if (throttle == 0 || percent == 0) {
        return throttle;
    }

    const uint16_t reduction = ((uint16_t)throttle * (uint16_t)percent) / 100U;
    if (throttle <= reduction) {
        return 0;
    }

    return (uint8_t)(throttle - reduction);
}

/**
 * Makes sure the throttle value does not increase too much once
 */
static uint8_t validateThrottleValue(uint8_t newThrottleValue, uint8_t lastThrottleValue)
{
    int16_t diff = (int16_t)newThrottleValue - (int16_t)lastThrottleValue;

    // Throttle is being decreased
    if (diff < 0) {
        // Apply smoothened throttle if the change is too large
        if (abs(diff) > maxThrottleChange) {
            return (uint8_t)(lastThrottleValue - maxThrottleChange);
        }
        // Otherwise return the requested value
        return newThrottleValue;
    }

    // Throttle is being increased
    if (diff > maxThrottleChange) {
        // Apply smoothened throttle if the change is too large
        return (uint8_t)(lastThrottleValue + maxThrottleChange);
    }

    // Otherwise return the requested value
    return newThrottleValue;
}

/**
 * BLE throttle value is between 0 and 255. So the resturned value
 * should be something in that range. However, the throttle should
 * also not increase more than x % at a time. So we should smoothen
 * the throttle here if the value is increasing or decreasing too
 * quickly. The throttle value should also take battery protection
 * flags into accoutn and reduce throttle in certain situations.
 * @return valid throttle value between 0 and 255
 */
uint8_t getValidThrottle(void)
{
    static uint8_t lastThrottleValue = 0;
    uint8_t newThrottleValue = (uint8_t)inputSignal_bleThrottle;
    uint8_t validThrottleValue = validateThrottleValue(newThrottleValue,
                                                      lastThrottleValue);

    // Update the lastThrottleValue to last valid value
    lastThrottleValue = validThrottleValue;
    return validThrottleValue;
}

/**
 * Apply the battery protection flags to the throttled value.
 * When a battery warning or hard-stop signal is active, the
 * throttle is cut back step by step so the next motor command
 * stays below the dangerous current/temperature level.
 * @return adjusted throttle value if any protection signals
 *         were active
 */
uint8_t getProtectedThrottle(void)
{
    // First get a valid throttle value
    uint8_t requestedThrottle = getValidThrottle();

    // Check if any battery flags are active
    const bool batteryProtectionActive =
        outputSignal_battery1_currentIsGettingTooHigh ||
        outputSignal_battery1_currentIsTooHigh ||
        outputSignal_battery2_currentIsGettingTooHigh ||
        outputSignal_battery2_currentIsTooHigh ||
        outputSignal_battery1_temperatureTooHigh ||
        outputSignal_battery2_temperatureTooHigh ||
        outputSignal_battery1_voltageDifferenceTooHigh ||
        outputSignal_battery2_voltageDifferenceTooHigh;

    // If no flags active, then the valid throttle is used as is
    if (!batteryProtectionActive) {
        return requestedThrottle;
    }

    // Handle case when the last throttle value is 0
    uint8_t protectedThrottle = outputSignal_motor1_throttle;
    if (protectedThrottle == 0) {
        protectedThrottle = requestedThrottle;
    }

    // Base reduction on battery flags is 2%
    uint8_t reductionPercent = 2;
    // If current is already too high (not getting there but already is)
    // The reduce the throttle 10%
    if (outputSignal_battery1_currentIsTooHigh ||
        outputSignal_battery2_currentIsTooHigh) {
        reductionPercent = 10;
    } else if (outputSignal_battery1_temperatureTooHigh ||
               outputSignal_battery2_temperatureTooHigh ||) {
        // If temps get out of hand, block the throttle entirely.
        // Takes a long time to cool batteries in tight space with
        // no active cooling.
        reductionPercent = 100;
    } else if (outputSignal_battery1_voltageDifferenceTooHigh ||
               outputSignal_battery2_voltageDifferenceTooHigh) {
        // Reduce by 25% if cell voltage differences are too big
        reductionPercent = 25;
    }

    // Apply reduction to the throttle signal
    protectedThrottle = applyReduction(protectedThrottle, reductionPercent);

    // The set throttle must never exceed the user-requested value while
    // protection is active, otherwise the next cycle can easily re-trigger
    // the same warning condition again.
    if (protectedThrottle > requestedThrottle) {
        protectedThrottle = requestedThrottle;
    }

    // Validate every throttle value, including the protected one. This ensures
    // that the motor is not accelerated into full stop in an instant which might
    // break the motor, impellers, shafts, or the shaft connections.
    return validateThrottleValue(protectedThrottle, outputSignal_motor1_throttle);
}