idf_component_register(SRCS "motorControl.c" "esc.c" "motorControl_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler
                    PRIV_REQUIRES esp_driver_ledc)