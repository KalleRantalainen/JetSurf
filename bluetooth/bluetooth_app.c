#include "bluetooth_app.h"

#include "bleMaster.h"
#include "bleMobileInterface.h"
#include "inputSignals.h"
#include "logger.h"

void bluetooth_appInitAll(void)
{
    bleMaster_init();
    bleMobileInterface_init(bleMaster_startScan);
}

void bluetooth_appCyclicEntryPoint(void)
{   
    // BLE throttle value ranges from 0 to 255 (one byte)
    inputSignal_bleThrottle = bleMaster_getThrottle();
    LOG_SIGNAL("BLE input throttle: %d\n", inputSignal_bleThrottle);
}