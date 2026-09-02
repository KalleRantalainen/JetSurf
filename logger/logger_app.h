#ifndef LOGGER_APP_H_
#define LOGGER_APP_H_

// Called periodically from the logger thread loop.
void logger_appCyclicEntryPoint(void);
// Called once when the logger thread starts.
void logger_appInitAll(void);

#endif // LOGGER_APP_H_