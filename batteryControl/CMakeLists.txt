idf_component_register(SRCS "batteryControl.c" "canHelpers.c" "batteryControl_app.c" "battery.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler logger)