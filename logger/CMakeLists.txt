idf_component_register(SRCS "logger_app.c" "logger.c" "sdCardModule.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_timer fatfs sdmmc esp_driver_spi)

# Logger does not use itself to log messages, however LOG_TAG is
# still defined to avoid warnings.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LOG_TAG=\"logger\"
)