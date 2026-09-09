idf_component_register(SRCS "neoM9Ngps.c" "positionTracking_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES logger
                    PRIV_REQUIRES esp_driver_uart)