#include "bluetooth_app.h"

#include "bleMaster.h"
#include "inputSignals.h"
#include "logger.h"

void bluetooth_appInitAll(void)
{
    bleMaster_init();
}

void bluetooth_appCyclicEntryPoint(void)
{   
    // BLE throttle value ranges from 0 to 255 (one byte)
    inputSignal_bleThrottle = bleMaster_getThrottle();
    LOG_INFO("Throttle received over BLE: %d\n", inputSignal_bleThrottle);
}