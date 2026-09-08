idf_component_register(SRCS "logger_app.c" "logger.c" "sdCardModule.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_timer fatfs sdmmc esp_driver_spi)