#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdbool.h>
#include <stdint.h>

// Log levels used by all modules.
#define LOG_LEVEL_INFO    0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_ERROR   2

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
void logger_init(void);
void logger_deinit(void);

// Thread-safe log API, used from any application module.
// Example: logLine(LOG_LEVEL_INFO, "motorControl", "motor rpm is: %d", rpm);
bool logLine(log_level_t level, const char *source, const char *fmt, ...);

// Drain the queued log entries and send them to the sink.
// The current sink is stdout; later this can be replaced by SD-card writing.
void logger_drainQueue(void);

// Macros can be called to automatically handle the log level
#define LOG_INFO(source, fmt, ...) logLine(LOG_LEVEL_INFO, source, fmt, ##__VA_ARGS__)
#define LOG_WARN(source, fmt, ...) logLine(LOG_LEVEL_WARNING, source, fmt, ##__VA_ARGS__)
#define LOG_ERR(source, fmt, ...) logLine(LOG_LEVEL_ERROR, source, fmt, ##__VA_ARGS__)

#endif // LOGGER_H_