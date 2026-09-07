#include "bluetooth_app.h"

#include "bleMaster.h"
#include "inputSignals.h"

void bluetooth_appInitAll(void)
{
    bleMaster_init();
}

void bluetooth_appCyclicEntryPoint(void)
{
    inputSignal_throttle = bleMaster_getThrottle();
}