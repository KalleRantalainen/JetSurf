#ifndef MOTORCONTROL_APP_H_
#define MOTORCONTROL_APP_H_

// Called periodically from the real time control loop
void motorControl_appCyclicEntryPoint(void);
// Called once when the application starts
void motorControl_appInitAll(void);

#endif // MOTORCONTROL_APP_H_