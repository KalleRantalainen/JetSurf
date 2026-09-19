#include "ledControl.h"

#include "led.h"
#include "outputSignals.h"

static led gpsLed;
static led sdLed;
static led bleThrottleLed;

/**
 * Initialize all HMI leds
 * @return void
 */
void initializeLeds(void)
{
    // Init all leds
    initLed(&gpsLed, 32);
    initLed(&sdLed, 33);
    initLed(&bleThrottleLed, 14);
}

/**
 * One cycle of computation for leds. For example, turn
 * blinking on or off.
 * @return void
 */
void cycleLeds(void)
{
    // Check if some leds should be turned on or off
    if (outputSignal_gpsHasFix) {
        startBlink(&gpsLed, 10);
    } else {
        stopBlink(&gpsLed);
    }

    if (outputSignal_sdCardWritingOk) {
        startBlink(&sdLed, 10);
    } else {
        stopBlink(&sdLed);
    }

    if (outputSignal_bleThrottleOk) {
        startBlink(&bleThrottleLed, 10);
    } else {
        stopBlink(&bleThrottleLed);
    }

    // Cycle the led animations
    blinker(&gpsLed);
    blinker(&sdLed);
    blinker(&bleThrottleLed);
}

