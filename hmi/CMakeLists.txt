idf_component_register(SRCS "led.c" "hmi_app.c" "led.c" "ledIndicator.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver logger signalHandler)

# Define LOG_TAG macro for every source file so that we do
# not have to define it manually in every file.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"hmi\"
)