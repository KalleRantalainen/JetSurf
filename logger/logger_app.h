#ifndef LOGGER_APP_H_
#define LOGGER_APP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Called periodically from the logger thread loop.
void logger_appCyclicEntryPoint(void);
// Called once when the logger thread starts.
void logger_appInitAll(void);

// Stop the logger task and close its output devices.
void logger_appStop(void);

// Reopen the logger output devices and resume the logger task.
void logger_appStart(void);

void logger_appSetTaskHandle(TaskHandle_t taskHandle);

#endif // LOGGER_APP_H_