#ifndef HMI_APP_H_
#define HMI_APP_H_

// Called periodically from the real time control loop
void hmi_appCyclicEntryPoint(void);
// Called once when the application starts
void hmi_appInitAll(void);

#endif // HMI_APP_H_