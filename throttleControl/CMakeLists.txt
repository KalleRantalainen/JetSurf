idf_component_register(SRCS "potentioMeter.c" "throttleControl_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler
                    PRIV_REQUIRES esp_driver_adc)