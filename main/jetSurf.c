#include <stdio.h>

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motorControl_app.h"
#include "throttleControl_app.h"

// TODO: Create two threads.
// Thread 1: Should run the real time application loop
// Thread 2: Should run logging loop and maybe handle any 
//           BLE <-> Mobile connection?

// Application cycle in milliseconds
static const int APPLICATION_CYCLE_MS = 250;

/**
 * Real-time control loop. This is run exactly once every
 * application cycle, which is defined by APPLICATION_CYCLE_MS
 */
static void applicationTimerCallback(void *arg)
{
    (void)arg;

    motorControl_appCyclicEntryPoint();
    throttleControl_appCyclicEntryPoint();
}

/**
 * Initialize all the applications
 */
static void applicationInit(void)
{
    motorControl_appInitAll();
    throttleControl_appInitAll();
}

/**
 * Starting point of the whole software
 */
void app_main(void)
{
    // Initialize all the applications first
    applicationInit();

    // Create a periodic timer for the real time control loop
    esp_timer_handle_t periodicTimer;
    const esp_timer_create_args_t timerArgs = {
        .callback = &applicationTimerCallback,
        .arg = NULL,
        .name = "applicationTimer"
    };
    printf("Starting control loop!\n");
    ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &periodicTimer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodicTimer, APPLICATION_CYCLE_MS * 1000));
    printf("Control loop started!\n");

    // Keep-alive loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
