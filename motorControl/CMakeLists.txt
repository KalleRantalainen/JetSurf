idf_component_register(SRCS "motorControl.c" "esc.c" "motorControl_app.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_ledc)