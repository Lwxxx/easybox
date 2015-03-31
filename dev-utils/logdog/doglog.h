#ifndef __DOG_LOG_H__
#define __DOG_LOG_H__

#include <stdio.h>
#include <string.h>

/**
 * log of doglog itself
 */

#ifdef LOG_USE_SYSLOG
#define log(fmt, ..) do {                       \
    openlog("logdog", LOG_CONS | LOG_PID, 0);   \
    syslog(LOG_DEBUG, fmt, ##__VA_ARGS__);      \
    closelog();                                 \
}
#else
#define log printf
#endif // LOG_USE_SYSLOG


#define LOGDOG_DEBUG
#ifdef LOGDOG_DEBUG
#define LDG_LOG(fmt, ...) do {     \
    log(fmt, ##__VA_ARGS__);       \
} while(0)
#else
#define LDG_LOG(fmt, ...)
#endif // LOGDOG_DEBUG

#define LDG_ERR(fmt, ...) do {     \
       log(fmt, ##__VA_ARGS__);    \
} while(0)

#endif // __DOG_LOG_H__
