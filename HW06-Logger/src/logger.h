#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

#define LOG_DEBUG(fmt, ...) logger_log(DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) logger_log(INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) logger_log(WARNING, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger_log(ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR
} log_level_t;

_Bool logger_init(const char *log_file_path);
void logger_log(log_level_t level, const char *file, int line, const char *func, const char *format, ...) __attribute__((format(printf, 5, 6)));
void logger_cleanup();


#endif // LOGGER_H
