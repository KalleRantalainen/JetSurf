#include <stdio.h>

#include "motorControl_app.h"

#include "motorControl.h"
#include "outputSignals.h"
#include "logger.h"

static Motor leftMotor;  // Motor 1, connected to battery 1 "grayBattery"
static Motor rightMotor; // Motor 2, connected to battery 2 "blueBattery"

/**
 * Read all application specific signals periodically
 */
static void readAll(void)
{
    // Motor control has no read signals.
}

/**
 * Write all application speicific signals periodically
 */
static void writeAll(void)
{
    // Repeatedly set the motor speed.
    setSpeed(outputSignal_motor1_throttle, &leftMotor);
    setSpeed(outputSignal_motor2_throttle, &rightMotor);
}

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void motorControl_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called in the beginning
 */
void motorControl_appInitAll(void)
{
    // Connect left motor to gpio pin 25
    initMotor(25, &leftMotor);
    // Connect right motor to gpio pin 26
    initMotor(26, &rightMotor);

    // Set motor speed to 0 intially
    setSpeed(0, &leftMotor);
    setSpeed(0, &rightMotor);
}

