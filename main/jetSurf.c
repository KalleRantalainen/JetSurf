#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motorControl_app.h"
#include "throttleControl_app.h"
#include "batteryControl_app.h"
#include "canHelpers.h"
#include "logger_app.h"
#include "bluetooth_app.h"
#include "positionTracking_app.h"

#include "logger.h"

// TODO: Create two threads.
// Thread 1: Should run the real time application loop
// Thread 2: Should run logging loop and maybe handle any 
//           BLE <-> Mobile connection?

// Application cycle in milliseconds
static const int APPLICATION_CYCLE_MS = 250;
static esp_timer_handle_t s_periodicTimer;

/**
 * Real-time control loop. This is run exactly once every
 * application cycle, which is defined by APPLICATION_CYCLE_MS
 */
static void applicationTimerCallback(void *arg)
{
    (void)arg;

    motorControl_appCyclicEntryPoint();
    bluetooth_appCyclicEntryPoint();
    throttleControl_appCyclicEntryPoint();
    batteryControl_appCyclicEntryPoint();
    positionTracking_appCyclicEntryPoint();
}

/**
 * Run logger task indefinitely. Write the logs periodically
 * using the application cycle as the period.
 */
static void loggerTask(void *arg)
{
    (void)arg;

    logger_appInitAll();

    while (1) {
        logger_appCyclicEntryPoint();
        vTaskDelay(pdMS_TO_TICKS(APPLICATION_CYCLE_MS));
    }
}

/**
 * Poll monitor input without blocking and request a new SD-card log file when 's' is received.
 */
static void logCommandTask(void *arg)
{
    (void)arg;

    const int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    printf("Serial logging control: send 's' to start a new log file.\n");
    while (1) {
        char command;
        const ssize_t bytesRead = read(STDIN_FILENO, &command, 1);
        if (bytesRead == 1 && (command == 's' || command == 'S')) {
            logger_requestRotation();
            printf("Log rotation requested.\n");
        } else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * Initialize all the applications
 */
static void applicationInit(void)
{
    motorControl_appInitAll();
    bluetooth_appInitAll();
    throttleControl_appInitAll();
    batteryControl_appInitAll();
    positionTracking_appInitAll();
}

/**
 * Starting point of the whole software
 */
void app_main(void)
{
    // Initialize all the applications first
    applicationInit();

    // Start the logging task. This keeps the real-time timer separate from the queue drain.
    if (xTaskCreate(&loggerTask, "loggerTask", 4096, NULL, 5, NULL) != pdPASS) {
        printf("Failed to create logger task.\n");
        return;
    }

    if (xTaskCreate(&logCommandTask, "logCommandTask", 3072, NULL, 5, NULL) != pdPASS) {
        printf("Failed to create log command task.\n");
        return;
    }

    // Create a periodic timer for the real time control loop
    const esp_timer_create_args_t timerArgs = {
        .callback = &applicationTimerCallback,
        .arg = NULL,
        .name = "applicationTimer"
    };
    printf("Starting control loop!\n");
    ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &s_periodicTimer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_periodicTimer, APPLICATION_CYCLE_MS * 1000));
    printf("Control loop started!\n");

    // Keep-alive loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
