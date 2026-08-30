#include <stdio.h>

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app.h"

// Application cycle in milliseconds
static const int APPLICATION_CYCLE_MS = 250;

static void applicationTimerCallback(void *arg)
{
    (void)arg;

    motorControl_appCyclicEntryPoint();
}

static void applicationInit(void)
{
    motorControl_appInitAll();
}

void app_main(void)
{
    applicationInit();

    esp_timer_handle_t periodicTimer;
    const esp_timer_create_args_t timerArgs = {
        .callback = &applicationTimerCallback,
        .arg = NULL,
        .name = "applicationTimer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &periodicTimer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodicTimer, APPLICATION_CYCLE_MS * 1000));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
