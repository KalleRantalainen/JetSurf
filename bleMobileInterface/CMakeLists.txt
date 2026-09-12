idf_component_register(SRCS "bleMobileInterface.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt nvs_flash signalHandler logger freertos)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"bleMobileInterface\"
)
