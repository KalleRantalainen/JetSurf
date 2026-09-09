#ifndef POSITIONTRACKING_APP_H_
#define POSITIONTRACKING_APP_H_

// Called periodically from the real time control loop
void positionTracking_appCyclicEntryPoint(void);
// Called once when the application starts
void positionTracking_appInitAll(void);

#endif // POSITIONTRACKING_APP_H_