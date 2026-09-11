idf_component_register(SRCS "motorControl.c" "esc.c" "motorControl_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler logger
                    PRIV_REQUIRES esp_driver_ledc)

# Define LOG_TAG macro for every source file so that we do
# not have to define it manually in every file.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"motorControl\"
)