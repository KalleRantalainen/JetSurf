#include "sdCardModule.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

static const char *TAG = "sdCardModule";
static FILE *s_logFile = NULL;
static sdmmc_card_t *s_card = NULL;
static spi_host_device_t s_hostId;

/**
 * Find the next available numbered log file.
 * @return one greater than the highest numbered log file on the SD card, or 1 if none exist
 */
static int findNextLogNumber(void)
{
	DIR *directory = opendir(SD_CARD_MOUNT_POINT);
	if (directory == NULL) {
		return 1;
	}

	int highestNumber = 0;
	struct dirent *entry;
	// Inspect only filenames matching the log<number>.log naming pattern.
	while ((entry = readdir(directory)) != NULL) {
		int number;
		char suffix;
		if (sscanf(entry->d_name, "log%d.log%c", &number, &suffix) == 1 && number > highestNumber) {
			highestNumber = number;
		}
	}
	closedir(directory);
	return highestNumber + 1;
}

/**
 * Initialize the SD card and create the next numbered log file.
 * @return true if the SD card and log file were initialized successfully; otherwise false
 */
bool sdCardModule_init(void)
{
	if (s_logFile != NULL) {
		return true;
	}

	spi_bus_config_t busConfig = {
		.mosi_io_num = SD_CARD_MOSI_GPIO,
		.miso_io_num = SD_CARD_MISO_GPIO,
		.sclk_io_num = SD_CARD_SCK_GPIO,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	s_hostId = host.slot;

	// Initialize the shared SPI bus before connecting the SD card device.
	esp_err_t error = spi_bus_initialize(s_hostId, &busConfig, SDSPI_DEFAULT_DMA);
	if (error != ESP_OK) {
		ESP_LOGE(TAG, "SPI bus initialization failed: %s", esp_err_to_name(error));
		return false;
	}

	sdspi_device_config_t slotConfig = SDSPI_DEVICE_CONFIG_DEFAULT();
	slotConfig.gpio_cs = SD_CARD_CS_GPIO;
	slotConfig.host_id = s_hostId;

	const esp_vfs_fat_mount_config_t mountConfig = {
		.format_if_mount_failed = false,
		.max_files = 4,
		.allocation_unit_size = 16 * 1024,
	};
	error = esp_vfs_fat_sdspi_mount(SD_CARD_MOUNT_POINT, &host, &slotConfig, &mountConfig, &s_card);
	if (error != ESP_OK) {
		ESP_LOGE(TAG, "SD card mount failed: %s", esp_err_to_name(error));
		spi_bus_free(s_hostId);
		s_card = NULL;
		return false;
	}

	// Create a new file so every software run has its own log.
	char path[64];
	snprintf(path, sizeof(path), SD_CARD_MOUNT_POINT "/log%d.log", findNextLogNumber());
	s_logFile = fopen(path, "w");
	if (s_logFile == NULL) {
		ESP_LOGE(TAG, "Could not create %s", path);
		sdCardModule_deinit();
		return false;
	}

	ESP_LOGI(TAG, "Logging to %s", path);
	return true;
}

/**
 * Write data to the current SD card log file.
 * @param data buffer containing the data to write
 * @param length number of bytes to write from data
 * @return true if all data was written and flushed successfully; otherwise false
 */
bool sdCardModule_write(const char *data, size_t length)
{
	if (s_logFile == NULL || data == NULL || length == 0) {
		return false;
	}

	if (fwrite(data, 1, length, s_logFile) != length) {
		return false;
	}
	// Flush each write so log data survives a reset or unexpected shutdown.
	return fflush(s_logFile) == 0;
}

/**
 * Close the current log file and unmount the SD card.
 * @return void
 */
void sdCardModule_deinit(void)
{
	if (s_logFile != NULL) {
		fclose(s_logFile);
		s_logFile = NULL;
	}
	if (s_card != NULL) {
		esp_vfs_fat_sdcard_unmount(SD_CARD_MOUNT_POINT, s_card);
		s_card = NULL;
		spi_bus_free(s_hostId);
	}
}