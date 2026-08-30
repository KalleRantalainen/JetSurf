#include "esc.h"

#include "driver/ledc.h"
#include "esp_err.h"


// The LEDC peripheral needs a timer to be configured before individual channels can
// be created. This helper makes sure the shared timer is initialized only once.
static bool s_timerConfigured = false;

/**
 * Configures LEDC timer for the all ESC outputs. The Flier ESCs expect a
 * 50 Hz signal with pulse widths between 1 ms (0% throttle) and 2 ms (100% throttle).
 * 
 */
static void configureLedcTimerIfNeeded(void)
{
    if (s_timerConfigured) {
        return;
    }

    const ledc_timer_config_t timerConfig = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&timerConfig);
    if (err != ESP_OK) {
        s_timerConfigured = false;
        return;
    }

    s_timerConfigured = true;
}

/**
 * Initialize one ESC instance for a selected GPIO pin.
 * @param esc the ESC instance to be initialized
 * @param gpio gpio pin number. The ESC signal wire should
 * be connected to this.
 */
void initEsc(ESC *esc, int gpio)
{   
    // persists between calls; next free LEDC channel
    static int s_nextChannel = 0;

    if (esc == NULL) {
        return;
    }

    esc->gpioPin = gpio;
    esc->initialized = false;

    // assign this ESC to the current free channel
    esc->pwmChannel = s_nextChannel;

    // shared timer for all ESC outputs
    configureLedcTimerIfNeeded();

    if (s_nextChannel >= LEDC_CHANNEL_MAX) {
        return;
    }

    const ledc_channel_config_t channelConfig = {
        .gpio_num = gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)s_nextChannel, // this channel is tied to the shared timer below
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };

    esp_err_t err = ledc_channel_config(&channelConfig);
    if (err == ESP_OK) {
        esc->initialized = true;
        // Increase the channel
        s_nextChannel++;
    }
}

/**
 * Set the commanded PWM pulse width in microseconds.
 *
 * ESCs are servo-like devices: a 1 ms pulse means minimum throttle, 2 ms means full
 * throttle. The LEDC duty value is not a microsecond
 * count directly, so we map the requested pulse width to the hardware timer range.
 * 
 * @param esc the esc which pulse width we want to change
 * @param pulseWidthUs new pulse width value for the esc in microseconds
 * @return true if everything is successful
 */
bool escSetPulseWidthMicroseconds(ESC *esc, int pulseWidthUs)
{
    if (esc == NULL || !esc->initialized) {
        return false;
    }

    // The pulse width has to be between the min and max values.
    // If not, then force it to the nearest limit.
    if (pulseWidthUs < ESC_PULSE_WIDTH_US_MIN) {
        pulseWidthUs = ESC_PULSE_WIDTH_US_MIN;
    } else if (pulseWidthUs > ESC_PULSE_WIDTH_US_MAX) {
        pulseWidthUs = ESC_PULSE_WIDTH_US_MAX;
    }

    /*
     * LEDC is configured with 14-bit resolution, so the maximum duty is 2^14 - 1.
     * The ESC timing range is 1 ms to 2 ms, which is why we compare against 20000 us
     * (20 ms period / 50 Hz). This keeps the duty cycle proportional to the desired pulse
     * width while constraining it to the valid range.
     */

    // Maximym duty is 2^14 - 1 = 16383 because we have 14 bit resolution
    const uint32_t maxDuty = (1UL << LEDC_TIMER_14_BIT) - 1UL;

    // Convert the desired pulse width (µs) into the corresponding PWM duty value.
    const uint32_t pulseDuty = (uint32_t)((uint64_t)pulseWidthUs * maxDuty / 20000ULL);

    // Set the new duty value for the ESC's PWM channel.
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)esc->pwmChannel, pulseDuty);
    if (err != ESP_OK) {
        return false;
    }

    // Apply the new duty value to the PWM hardware.
    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)esc->pwmChannel);
    if (err != ESP_OK) {
        return false;
    }

    return true;
}