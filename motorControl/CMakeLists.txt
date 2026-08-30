idf_component_register(SRCS "motorControl.c" "esc.c" "app.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_ledc)