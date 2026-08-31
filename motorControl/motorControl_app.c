#include "motorControl_app.h"

#include "motorControl.h"

static Motor leftMotor;
static Motor rightMotor;

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
    // (set to 0 FOR NOW)
    setSpeed(0, &leftMotor);
    setSpeed(0, &rightMotor);
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