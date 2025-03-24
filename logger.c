#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/*
 * Functions to satisfy the assignment
 * */
static void log_with_level(const char *level, const char *fmt, va_list args) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    printf("[%02d:%02d:%02d] %s: ", local->tm_hour, local->tm_min, local->tm_sec, level);
    vprintf(fmt, args);
    printf("\n");
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_with_level("INFO", fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_with_level("ERROR", fmt, args);
    va_end(args);
}


//
// Example of using vfprintf for safe logging
//
void log_info_safe(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    printf("\n");
}