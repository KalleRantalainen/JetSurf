idf_component_register(SRCS "jetSurf.c"
                    INCLUDE_DIRS "."
                    REQUIRES motorControl throttleControl logger batteryControl bluetooth esp_timer positionTracking)

# Define LOG_TAG macro for every source file so that we do
# not have to define it manually in every file.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"main\"
)