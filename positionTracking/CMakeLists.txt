idf_component_register(SRCS "neoM9Ngps.c" "positionTracking_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES logger signalHandler
                    PRIV_REQUIRES esp_driver_uart esp_driver_gpio esp_timer)

# Define LOG_TAG macro for every source file so that we do
# not have to define it manually in every file.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"positionTracking\"
)