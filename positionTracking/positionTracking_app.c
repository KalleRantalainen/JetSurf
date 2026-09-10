#include <stdio.h>

#include "positionTracking_app.h"

#include "inputSignals.h"
#include "logger.h"
#include "neoM9Ngps.h"


/**
 * Read all application specific signals periodically
 */
static void readAll(void)
{
    gps_position_t position;
    readPosition();
    if (neoM9Ngps_getPosition(&position)) {
        LOG_INFO("positionTracking", "GPS position: latitude=%.6f, longitude=%.6f, speed=%.2f m/s",
                 position.latitudeDegrees, position.longitudeDegrees,
                 position.speedMetersPerSecond);
        LOG_INFO("positionTracking", "GSP course: %.6f, fixTimestamp: %d", position.courseDegrees,
            position.fixTimestampMs);
    } else {
        LOG_INFO("positionTracking", "GPS has no fix.");
    }
}

/**
 * Write all application speicific signals periodically
 */
static void writeAll(void)
{
    // No writes at the moment
}

/**
 * Entry for the main interrupt loop. Does everything the
 * applications has to do periodically
 */
void positionTracking_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called in the beginning
 */
void positionTracking_appInitAll(void)
{
    initGps();
}