idf_component_register(SRCS "outputSignals.c" "inputSignals.c"
                    INCLUDE_DIRS "."
                    )

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"signalHandler\"
)