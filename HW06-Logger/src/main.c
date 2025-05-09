#include <stdlib.h>
#include <stdio.h>

#include "logger.h"

int main(void)
{
    _Bool result = logger_init("app.log");

    if (result) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    LOG_INFO("Application started successfully.");
    LOG_DEBUG("This is a debug message.");
    LOG_WARNING("Warning message example.");
    LOG_ERROR("Error occurred!");

    logger_cleanup();
    return 0;
}
