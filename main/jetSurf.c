#include <stdio.h>

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Application cycle in milliseconds
static const int APPLICATION_CYCLE_MS = 250;

static void application_timer_callback(void *arg)
{
    // Call applications from here
    printf("Application tick\n");
}

void app_main(void)
{
    // Create a timer handle
    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t timer_args = {
        .callback = &application_timer_callback,
        .arg = NULL,
        .name = "application_timer"
    };

    // Create a periodic timer calling the application timer
    // callback with a cycle of APPLICATION_CYCLE_MS
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, APPLICATION_CYCLE_MS * 1000));

    // Keep the app alive while the timer runs in the background.
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
