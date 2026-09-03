idf_component_register(SRCS "jetSurf.c"
                    INCLUDE_DIRS "."
                    REQUIRES motorControl throttleControl logger batteryControl esp_timer)
