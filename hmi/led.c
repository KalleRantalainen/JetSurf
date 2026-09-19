#include "led.h"

#include "driver/gpio.h"

/**
 * Lower level local function to set
 * the gpio high if low and low if high.
 * @return void
 */
static void toggleGpio(led* led)
{
	if (led == NULL || !led->initialized) {
		return;
	}

    // Read the current gpio level
	const int currentLevel = gpio_get_level((gpio_num_t)led->gpioPin);
    // Write the opposite level
	gpio_set_level((gpio_num_t)led->gpioPin, currentLevel == 0 ? 1 : 0);

    // Update the led's on status
    led->ledOn = !currentLevel;
}

/**
 * Initialized a led with a given GPIO pin
 * @return void
 */
void initLed(led* led, int gpioPin)
{
    if (led == NULL || gpioPin < 0 || gpioPin >= GPIO_NUM_MAX) {
		return;
	}

	led->gpioPin = gpioPin;
	led->initialized = false;
	led->blinking = false;
	led->onCycles = 0;
	led->offCycles = 0;
    led->ledOn = false;
    led->currentCycles = 0;

	const gpio_config_t config = {
		.pin_bit_mask = 1ULL << gpioPin,
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
	};

	if (gpio_config(&config) != ESP_OK) {
		return;
	}

	gpio_set_level((gpio_num_t)gpioPin, 0);
	led->initialized = true;
}

/**
 * Starts to blink the led with give cyclecount. Led
 * is on for cycleCount cycles and then off for
 * for the same time. Repeat this until stopBlink is called
 * @return void
 */
void startBlink(led* led, int cycleCount)
{
    if (led == NULL || !led->initialized || cycleCount <= 0) {
        return;
    }

    // Set the blinking values, blinker handles blinking
    led->blinking = true;
    led->onCycles = cycleCount;
    led->offCycles = cycleCount;
}

/**
 * Stop the led from blinking if the led is blinking
 * @return void
 */
void stopBlink(led* led)
{
    if (led == NULL || !led->initialized) {
        return;
    }

    led->blinking = false;
}

/**
 * Blink the led according to their animation plan
 * @param led pointer to the led to be blinked
 * @return void
 */
void blinker(led* led)
{
    if (led == NULL || !led->initialized) {
        return;
    }

    if (led->blinking) {
        // Led is blinking, check if the gpio needs to be toggled
        if (led->ledOn) {
            // Led is on, check if it needs to be turned off
            if (led->currentCycles >= led->onCycles) {
                // Led has been on for long enough, turn it off
                // (toggle updates the ledOn param)
                toggleGpio(led);
            }
        } else {
            // Led is off, check if it needs to be turned on
            if (led->currentCycles >= led->offCycles) {
                // Led should be turned on
                toggleGpio(led);
            }
        }
    } else {
        // Led is not blinking. Led should always be on and blinking
        // or off and not blinking, never on and not blinking. If
        // the state the led is still on, it means that the blinkiung
        // was exited when the led was on. Turn it off
        if (led->ledOn) {
            toggleGpio(led);
        }
    }
    // Increment led cycles.
    led->currentCycles++;
}