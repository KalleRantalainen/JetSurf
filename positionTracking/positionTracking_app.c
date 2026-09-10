#include <stdio.h>

#include "positionTracking_app.h"

#include "inputSignals.h"
#include "logger.h"
#include "neoM9Ngps.h"

/**
 * Write all global signals of this application
 */
static void writeGlobal(void)
{
    gps_position_t position;
    if (neoM9Ngps_getPosition(&position)) {
        // Write some GPS signals as global signals for other applications
        inputSignal_velocityMetSec = position->speedMetersPerSecond;
        inputSignal_latitudeDeg = position->latitudeDegrees;
        inputSignal_longitudeDeg = position->longitudeDegrees;
        inputSignal_courseDeg = position->courseDegrees;
        inputSignal_gpsTimestampMs = position->fixTimestampMs;

        // Write the signals
        LOG_INFO("positionTracking", "GPS position: latitude=%.6f, longitude=%.6f, fixTimestamp: %d\n",
            inputSignal_latitudeDeg, inputSignal_longitudeDeg, inputSignal_gpsTimestampMs);
        LOG_INFO("positionTracking", "GPS velocity: %.2f m/s, fixTimestamp: %d\n",
            inputSignal_velocityMetSec, inputSignal_gpsTimestampMs);
        LOG_INFO("positionTracking", "GSP course: %.6f, fixTimestamp: %d\n", inputSignal_courseDeg,
            inputSignal_gpsTimestampMs);
    }
}

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void positionTracking_appCyclicEntryPoint(void)
{
    writeGlobal();
}

/**
 * Initialization function called in the beginning
 */
void positionTracking_appInitAll(void)
{
    initGps();
}