#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Log queue, all messages added to this queue first
static QueueHandle_t s_logQueue = NULL;

/**
 * Convert log level to its string presenstation
 * @param level log level as integer: 0, 1 or 2
 * @return level as string, 0 -> INFO, 1 -> WARNING, 2 -> ERROR
 */
static const char *logLevelToString(log_level_t level)
{
    switch (level) {
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARNING:
            return "WARNING";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "INFO";
    }
}

/**
 * Initialize the logger
 */
void logger_init(void)
{
    if (s_logQueue != NULL) {
        return;
    }

    // Create thread safe log queue for a maximum of 32 log lines
    s_logQueue = xQueueCreate(32, sizeof(log_entry_t));
}

/**
 * Destruct the logger
 */
void logger_deinit(void)
{
    if (s_logQueue == NULL) {
        return;
    }

    vQueueDelete(s_logQueue);
    s_logQueue = NULL;
}

/**
 * Add a single log line to the queue
 * @param level INFO, WARNING, ERROR
 * @param source string representation of the application, for example motorControl
 * @param fmt log message
 * @return true is success
 */
bool logLine(log_level_t level, const char *source, const char *fmt, ...)
{
    if (s_logQueue == NULL || source == NULL || fmt == NULL) {
        return false;
    }

    // Init log entry
    log_entry_t entry;
    memset(&entry, 0, sizeof(entry));

    // Set the entry source
    entry.level = level;
    snprintf(entry.source, sizeof(entry.source), "%s", source);

    // Set the entry message
    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    // Put the log line to the queue and return the status
    // of that.
    return xQueueSend(s_logQueue, &entry, 0) == pdTRUE;
}

/**
 * Drain the log queue to the log file
 */
void logger_drainQueue(void)
{
    if (s_logQueue == NULL) {
        return;
    }

    log_entry_t entry;
    while (xQueueReceive(s_logQueue, &entry, 0) == pdTRUE) {
        // This is the current sink. Later this can be swapped for SD-card logging.
        printf("[%s] [%s] %s\n", logLevelToString(entry.level), entry.source, entry.message);
    }
}