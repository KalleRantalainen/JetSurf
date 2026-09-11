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
    static uint32_t lastFixTimestamp = 0;
    gps_position_t position;

    // Read raw UART bytes and decode any complete NMEA sentence before
    // checking whether the GPS module currently has a valid fix.
    readPosition();

    if (neoM9Ngps_getPosition(&position)) {
        // Write some GPS signals as global signals for other applications
        inputSignal_velocityMetSec = position.speedMetersPerSecond;
        inputSignal_latitudeDeg = position.latitudeDegrees;
        inputSignal_longitudeDeg = position.longitudeDegrees;
        inputSignal_courseDeg = position.courseDegrees;
        inputSignal_gpsTimestampMs = position.fixTimestampMs;

        if (lastFixTimestamp != position.fixTimestampMs) {
            // LOG the signals if they have updated
            LOG_SIGNAL("Latitude: %.6f°\n", inputSignal_latitudeDeg);
            LOG_SIGNAL("Longitude: %.6f°\n", inputSignal_longitudeDeg);
            LOG_SIGNAL("Velocity: %.2f m/s\n", inputSignal_velocityMetSec);
            LOG_SIGNAL("Course: %.6f°\n", inputSignal_courseDeg);
            LOG_SIGNAL("GPS timestamp: %d\n ms", inputSignal_gpsTimestampMs);
        }

        lastFixTimestamp = position.fixTimestampMs;
    } else {
        LOG_WARN("GPS has no fix\n");
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