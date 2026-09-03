idf_component_register(SRCS "batteryControl.c" "canHelpers.c" "batteryControl_app.c" "battery.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler logger driver esp_driver_twai esp_driver_gpio)