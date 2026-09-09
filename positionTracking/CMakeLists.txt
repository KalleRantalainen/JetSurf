idf_component_register(SRCS "neoM9Ngps.c" "positionTracking_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES logger signalHandler
                    PRIV_REQUIRES esp_driver_uart esp_driver_gpio esp_timer)