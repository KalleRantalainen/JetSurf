#include "motorControl.h"

#include "logger.h"

/**
 * Update the motor's speed.
 * @param speed motor speed value in the 0...255 range
 * @param motor pointer to the correct motor
 * @return true if speed setting was successful
 */
bool setSpeed(int speed, Motor *motor)
{
    if (motor == NULL) {
        return false;
    }

    // Speed value must be between 0 and 255, if not
    // then clamp it.
    int clampedSpeed = speed;
    if (clampedSpeed < motor->minSpeed) {
        clampedSpeed = motor->minSpeed;
    }
    if (clampedSpeed > motor->maxSpeed) {
        clampedSpeed = motor->maxSpeed;
    }

    // Convert the 0...255 speed value into a usable PWM pulse width.
    // PWM pulse width is between 1000 and 2000 microseconds.
    const int pulseWidthUs = ESC_PULSE_WIDTH_US_MIN +
        ((ESC_PULSE_WIDTH_US_MAX - ESC_PULSE_WIDTH_US_MIN) * clampedSpeed) /
        (motor->maxSpeed - motor->minSpeed);

    LOG_INFO("Final motor PWM signal: %d", pulseWidthUs);

    // Update the PWM signal
    return escSetPulseWidthMicroseconds(&motor->esc, pulseWidthUs);
}

/**
 * Initialize a motor when the application starts.
 * @param gpioPin the pin to which the ESC's signal wire is connected to
 * @param motor pointer to the correct motor (left or right)
 */
void initMotor(int gpioPin, Motor *motor)
{
    if (motor == NULL) {
        return;
    }

    // Use the same 0...255 convention as the rest of the project.
    motor->minSpeed = 0;
    motor->maxSpeed = 255;
    // Connect to the correct gpio pin
    initEsc(&motor->esc, gpioPin);

    // Set the initial speed to zero.
    if (motor->esc.initialized) {
        setSpeed(motor->minSpeed, motor);
    }
}