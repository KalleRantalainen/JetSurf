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
    inputSignal_throttle = bleMaster_getThrottle();
    LOG_INFO("bluetooth", "Throttle received over BLE: %d\n", inputSignal_throttle);
}