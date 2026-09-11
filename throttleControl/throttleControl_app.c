#include "throttleControl_app.h"
#include "throttleControl.h"

#include "inputSignals.h"
#include "outputSignals.h"

#include "logger.h"

/**
 * Write all application's global signals
 */
static void writeGlobal(void)
{
    // BLE throttle is between 0 and 255
    // Both motors receive the same throttle for now, so
    // there is no differential throttle for now. However,
    // both motors might not actually draw the same Amps in
    // reality due to worn down parts, or some shit in the
    // water inlet or the jet nozzle. This is why it migth
    // be a good idea to try to match the amps pulled from
    // the batteries rather than matching the throttle. This
    // way the throttle values would vary between the motors
    // but both motors would pull the same amps and they would
    // likely have close to same rpm?
    uint8_t throttle = getProtectedThrottle();
    outputSignal_motor1_throttle = throttle;
    outputSignal_motor2_throttle = throttle;

    LOG_SIGNAL("Throttle output motor1: %d\n", outputSignal_motor1_throttle);
    LOG_SIGNAL("Throttle output motor2: %d\n", outputSignal_motor2_throttle);
}

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void throttleControl_appCyclicEntryPoint(void)
{
    writeGlobal();
}

/**
 * Initialization function called in the beginning
 */
void throttleControl_appInitAll(void)
{
    // Initialize the global throttle signals to 0.
    outputSignal_motor1_throttle = 0;
    outputSignal_motor2_throttle = 0;
}