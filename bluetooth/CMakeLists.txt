idf_component_register(SRCS "bleMaster.c" "bluetooth_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler logger bt bleMobileInterface)

# Define LOG_TAG macro for every source file so that we do
# not have to define it manually in every file.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"bluetooth\"
)