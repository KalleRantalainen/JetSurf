#include "motorControl.h"

/**
 * Update the motor's speed
 * @param speed motor speed value in permilles, 0...1000
 * @param motor pointer to the correct motor
 * @return true if speed setting was successful
 */
bool setSpeed(int speed, Motor *motor)
{
    if (motor == NULL) {
        return false;
    }

    // Speed value must be between 0 and 1000, if not
    // then clamp it
    int clampedSpeed = speed;
    if (clampedSpeed < motor->minSpeed) {
        clampedSpeed = motor->minSpeed;
    }
    if (clampedSpeed > motor->maxSpeed) {
        clampedSpeed = motor->maxSpeed;
    }

    // Convert the arbitrary 0...1000 speed to an actual usable
    // PWM value. PWM value is betweeb 1000 and 2000
    const int pulseWidthUs = ESC_PULSE_WIDTH_US_MIN +
        ((ESC_PULSE_WIDTH_US_MAX - ESC_PULSE_WIDTH_US_MIN) * clampedSpeed) /
        (motor->maxSpeed - motor->minSpeed);

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

    // Minimum speed is 0 permilles 
    motor->minSpeed = 0;
    // Maximum speed is 1000 permilles
    motor->maxSpeed = 1000;
    // Connect to the correct gpio pin
    initEsc(&motor->esc, gpioPin);

    // Set the initial speed to zero.
    if (motor->esc.initialized) {
        setSpeed(motor->minSpeed, motor);
    }
}