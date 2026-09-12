#ifndef SD_CARD_MODULE_H_
#define SD_CARD_MODULE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Define SD card module pins
#ifndef SD_CARD_MISO_GPIO
#define SD_CARD_MISO_GPIO 19
#endif
#ifndef SD_CARD_MOSI_GPIO
#define SD_CARD_MOSI_GPIO 23
#endif
#ifndef SD_CARD_SCK_GPIO
#define SD_CARD_SCK_GPIO 18
#endif
#ifndef SD_CARD_CS_GPIO
#define SD_CARD_CS_GPIO 27
#endif

#define SD_CARD_MOUNT_POINT "/sdcard"
// Write maximum of 10MB per log file.
#define SD_CARD_MAX_LOG_FILE_SIZE (10U * 1024U * 1024U)

// Function to initialize the sd card
bool sdCardModule_init(void);
// Function to write data on the SD card
bool sdCardModule_write(const char *data, size_t length);
// Close the current log file and continue logging in the next file.
bool sdCardModule_rotate(void);
// Close the current log file and unmount sd card
void sdCardModule_deinit(void);
bool sdCardModule_isReady(void);
bool sdCardModule_getLatestSessionFiles(char filenames[][32], size_t maximumFiles, size_t *fileCount);
bool sdCardModule_readLatestSessionFile(const char *filename, uint32_t offset,
										void *buffer, size_t bufferSize, size_t *bytesRead);

#endif // SD_CARD_MODULE_H_
