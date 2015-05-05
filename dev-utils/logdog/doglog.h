#ifndef __DOG_LOG_H__
#define __DOG_LOG_H__

#include <stdio.h>
#include <string.h>

/**
 * log of doglog itself
 */
extern FILE* dog_log_file;

#define LOGDOG_DEBUG
#ifdef LOGDOG_DEBUG
#define LDG_LOG(fmt, ...) do {                          \
    if(dog_log_file) {                                  \
        fprintf(dog_log_file, fmt, ##__VA_ARGS__);      \
        fflush(dog_log_file);                           \
    }                                                   \
} while (0)
#else
#define LDG_LOG(fmt, ...)
#endif // LOGDOG_DEBUG

#define LDG_ERR(fmt, ...) do {                          \
    if(dog_log_file) {                                  \
        fprintf(dog_log_file, fmt, ##__VA_ARGS__);      \
        fflush(dog_log_file);                           \
    }                                                   \
} while (0)

#endif // __DOG_LOG_H__
