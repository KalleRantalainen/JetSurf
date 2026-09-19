#include "ledControl.h"

#include "led.h"

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

}

