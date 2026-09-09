#include "sdCardModule.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

static const char *TAG = "sdCardModule";
static FILE *s_logFile = NULL;
static sdmmc_card_t *s_card = NULL;
static spi_host_device_t s_hostId;
static unsigned int s_logFileNumber = 0;
static size_t s_logFileSize = 0;
static char s_sessionPath[64];

/**
 * Find the next available numbered session or log file.
 * @param directoryPath directory whose entries should be inspected
 * @param sessions true to search for session directories, false to search for log files
 * @return one greater than the highest matching number, or 1 if none exist
 */
static int findNextNumber(const char *directoryPath, bool sessions)
{
	DIR *directory = opendir(directoryPath);
	if (directory == NULL) {
		return 1;
	}

	int highestNumber = 0;
	struct dirent *entry;
	while ((entry = readdir(directory)) != NULL) {
		int number;
		char suffix;
		// The trailing character conversion rejects names with extra characters.
		const char *format = sessions ? "session%d%c" : "log%d.log%c";
		if (sscanf(entry->d_name, format, &number, &suffix) == 1 && number > highestNumber) {
			highestNumber = number;
		}
	}
	closedir(directory);
	return highestNumber + 1;
}

/**
 * Create and open the next log file in the current session.
 * @return true if the file was opened successfully; otherwise false
 */
static bool openNextLogFile(void)
{
	char path[96];
	snprintf(path, sizeof(path), "%s/log%d.log", s_sessionPath, s_logFileNumber);
	s_logFile = fopen(path, "w");
	if (s_logFile == NULL) {
		ESP_LOGE(TAG, "Could not create %s", path);
		return false;
	}
	s_logFileSize = 0;
	ESP_LOGI(TAG, "Logging to %s", path);
	return true;
}

/**
 * Initialize the SD card, create a new session directory, and open its first log file.
 * @return true if the SD card and log file were initialized successfully; otherwise false
 */
bool sdCardModule_init(void)
{
	if (s_logFile != NULL) {
		return true;
	}

	// Configure the ESP32 SPI pins used by the SD card module.
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

	// Mount without formatting so an initialization failure cannot erase the card.
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

	// Create one directory per firmware session so separate runs never share a log file.
	const unsigned int sessionNumber = (unsigned int)findNextNumber(SD_CARD_MOUNT_POINT, true);
	snprintf(s_sessionPath, sizeof(s_sessionPath), SD_CARD_MOUNT_POINT "/session%u", sessionNumber);
	if (mkdir(s_sessionPath, 0775) != 0) {
		ESP_LOGE(TAG, "Could not create %s", s_sessionPath);
		sdCardModule_deinit();
		return false;
	}

	// Start each session at log1.log; later files are created when the size limit is reached.
	s_logFileNumber = 1;
	if (!openNextLogFile()) {
		sdCardModule_deinit();
		return false;
	}
	return true;
}

/**
 * Append data to the current log file and rotate it when it reaches the size limit.
 * @param data buffer containing the data to write
 * @param length number of bytes to write from data
 * @return true if all data was written and synchronized successfully; otherwise false
 */
bool sdCardModule_write(const char *data, size_t length)
{
	if (s_logFile == NULL || data == NULL || length == 0) {
		return false;
	}

	if (s_logFileSize > 0 && s_logFileSize + length > SD_CARD_MAX_LOG_FILE_SIZE) {
		// Close the completed file before opening the next numbered file.
		if (fclose(s_logFile) != 0) {
			s_logFile = NULL;
			return false;
		}
		s_logFile = NULL;
		s_logFileNumber++;
		if (!openNextLogFile()) {
			return false;
		}
	}

	if (fwrite(data, 1, length, s_logFile) != length) {
		return false;
	}
	s_logFileSize += length;
	// Flush libc buffers and ask the filesystem to commit the write to the card.
	if (fflush(s_logFile) != 0) {
		return false;
	}
	return fsync(fileno(s_logFile)) == 0;
}

/**
 * Close the current log file and unmount the SD card.
 */
void sdCardModule_deinit(void)
{
	if (s_logFile != NULL) {
		fflush(s_logFile);
		fclose(s_logFile);
		s_logFile = NULL;
	}
	s_logFileSize = 0;
	if (s_card != NULL) {
		esp_vfs_fat_sdcard_unmount(SD_CARD_MOUNT_POINT, s_card);
		s_card = NULL;
		spi_bus_free(s_hostId);
	}
}