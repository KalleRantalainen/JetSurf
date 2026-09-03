#ifndef BATTERYCONTROL_APP_H_
#define BATTERYCONTROL_APP_H_

// Called periodically from the logger thread loop.
void batteryControl_appCyclicEntryPoint(void);
// Called once when the logger thread starts.
void batteryControl_appInitAll(void);

#endif // BATTERYCONTROL_APP_H_