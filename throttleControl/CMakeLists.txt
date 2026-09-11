idf_component_register(SRCS "potentioMeter.c" "throttleControl_app.c" "throttleControl.c"
                    INCLUDE_DIRS "."
                    REQUIRES signalHandler
                    PRIV_REQUIRES esp_driver_gpio esp_adc)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"throttleControl\"
)