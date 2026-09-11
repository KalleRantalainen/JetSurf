idf_component_register(SRCS "batteryControl.c" "canHelpers.c" "batteryControl_app.c" "battery.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler logger driver esp_driver_twai esp_driver_gpio esp_timer)

# Define LOG_TAG macro for every source file so that we do
# not have to define it manually in every file.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"batteryControl\"
)