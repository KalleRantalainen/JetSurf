#ifndef ESC_H_
#define ESC_H_

#include <stdbool.h>
#include <stdint.h>

// Flier boat ESC in forward only mode:
// - 0% throttle is 1000us PWM signal
// - 100% throttle signal is 2000us PWM signal
#define ESC_PULSE_WIDTH_US_MIN 1000
#define ESC_PULSE_WIDTH_US_MAX 2000

// ESC contains gpio pin number, pwm channel and init status
typedef struct {
    int gpioPin;
    int pwmChannel;
    bool initialized;
} ESC;

// Connect ESC to a gpio pin when the application starts
void initEsc(ESC *esc, int gpio);
// Update/set the ESC PWM signal
bool escSetPulseWidthMicroseconds(ESC *esc, int pulseWidthUs);

#endif // ESC_H_