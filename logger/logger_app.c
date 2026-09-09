#include <stdio.h>

#include "logger_app.h"

#include "logger.h"

#include "freertos/task.h"

static TaskHandle_t s_loggerTaskHandle = NULL;

/**
 * Read all signals periodically.
 */
static void readAll(void)
{
    // Logger has no read signals at the moment.
}

/**
 * Drain the queue and emit queued log lines to the configured sink.
 */
static void writeAll(void)
{
    logger_drainQueue();
}

/**
 * Logger loop calls this periodically.
 */
void logger_appCyclicEntryPoint(void)
{
    readAll();
    writeAll();
}

/**
 * Initialization function called when the logger thread starts.
 */
void logger_appInitAll(void)
{
    logger_init(LOGGER_OUTPUT_SD_CARD_AND_TERMINAL);
}

void logger_appStop(void)
{
    if (s_loggerTaskHandle != NULL) {
        vTaskSuspend(s_loggerTaskHandle);
    }
    logger_deinit();
}

void logger_appStart(void)
{
    logger_init(LOGGER_OUTPUT_SD_CARD_AND_TERMINAL);
    if (s_loggerTaskHandle != NULL) {
        vTaskResume(s_loggerTaskHandle);
    }
}

void logger_appSetTaskHandle(TaskHandle_t taskHandle)
{
    s_loggerTaskHandle = taskHandle;
}