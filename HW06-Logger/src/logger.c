#include <execinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdarg.h>

#include "logger.h"

static FILE *log_fp = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static _Bool initialized = 0;

static void print_stack_trace(int depth)
{
    void* buffer[depth];
    size_t count = backtrace(buffer, depth);
    char** symbols = backtrace_symbols(buffer, count);

    for (size_t i=0; i<count; ++i){
        fprintf(log_fp, "%s\n", symbols[i]);
    }

    free(symbols);
}

void logger_log(log_level_t level, const char *file, int line, const char *func, const char *format, ...)
{
    if (!initialized || !log_fp) {
        return;
    }

    va_list args;
    va_start(args, format);

    pthread_mutex_lock(&lock);

    switch(level) {
        case DEBUG:
            fprintf(log_fp, "[DEBUG] %s:%d (%s): ", file, line, func);
            break;
        case INFO:
            fprintf(log_fp, "[INFO] %s:%d (%s): ", file, line, func);
            break;
        case WARNING:
            fprintf(log_fp, "[WARN] %s:%d (%s): ", file, line, func);
            break;
        case ERROR:
            fprintf(log_fp, "[ERROR] %s:%d (%s): ", file, line, func);
            break;
    }

    vfprintf(log_fp, format, args);
    fprintf(log_fp, "\n");
    fflush(log_fp);

    if (level == ERROR) {
        print_stack_trace(10);
    }

    pthread_mutex_unlock(&lock);

    va_end(args);
}

_Bool logger_init(const char *log_file_path)
{
    if (initialized) {
        return 0;
    }

    log_fp = fopen(log_file_path, "a");
    if (!log_fp) {
        return 1;
    }

    initialized = 1;
    return 0;
}

void logger_cleanup()
{
    if (initialized && log_fp != NULL) {
        fclose(log_fp);
    }
    pthread_mutex_destroy(&lock);
    initialized = 0;
}
