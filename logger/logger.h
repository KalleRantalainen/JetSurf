#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdbool.h>
#include <stdint.h>

// Log levels used by all modules.
#define LOGGER_LEVEL_INFO    0
#define LOGGER_LEVEL_SIGNAL  1
#define LOGGER_LEVEL_WARNING 2
#define LOGGER_LEVEL_ERROR   3

// INFO: Contains informatic line for debugging
// SIGNAL: Signals are used to evaluate the performace
//         of the board when analyzing the log
// WARNING: Something has a value it should not have but
//          it causes no immediate harm
// ERROR: Something has a value is should not have and
//        it will cause some harm.

// Logger output destinations. Destinations can be combined when both outputs
// are required.
typedef enum {
    LOGGER_OUTPUT_TERMINAL,
    LOGGER_OUTPUT_SD_CARD,
    LOGGER_OUTPUT_SD_CARD_AND_TERMINAL
} logger_output_t;

// Define log level type related to each log line
typedef int log_level_t;

// Each log entry must contains this info
typedef struct {
    log_level_t level;
    char timestamp[32];
    char source[32];
    char message[192];
} log_entry_t;

// Queue lifecycle functions.
void logger_init(logger_output_t output);
void logger_deinit(void);

// Thread-safe log API, used from any application module.
// Example: logLine(LOGGER_LEVEL_INFO, "motorControl", "motor rpm is: %d", rpm);
bool logLine(log_level_t level, const char *source, const char *fmt, ...);

// Drain the queued log entries and send them to the configured sink.
void logger_drainQueue(void);

// Request a manual SD-card log rotation from the logger task.
void logger_requestRotation(void);

// Macros can be called to automatically handle the log level
#ifndef LOG_TAG
#define LOG_TAG "unknown"
#endif

#define LOG_INFO(fmt, ...) \
    logLine(LOGGER_LEVEL_INFO, LOG_TAG, fmt, ##__VA_ARGS__)

#define LOG_SIGNAL(fmt, ...) \
    logLine(LOGGER_LEVEL_SIGNAL, LOG_TAG, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    logLine(LOGGER_LEVEL_WARNING, LOG_TAG, fmt, ##__VA_ARGS__)

#define LOG_ERR(fmt, ...) \
    logLine(LOGGER_LEVEL_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)

#endif // LOGGER_H_