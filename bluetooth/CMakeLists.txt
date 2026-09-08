idf_component_register(SRCS "bleMaster.c" "bluetooth_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler logger bt nvs_flash)