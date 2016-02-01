#ifndef CCEPH_LOG_H
#define CCEPH_LOG_H

#include <stdarg.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LL_FATAL                 0 
#define LL_ERROR                 1 
#define LL_WARN                  2
#define LL_NOTICE                3
#define LL_INFO                  4
#define LL_DEBUG                 5

#define MSG_FATAL                LL_FATAL, __FILE__, __LINE__, __FUNCTION__
#define MSG_ERROR                LL_ERROR, __FILE__, __LINE__, __FUNCTION__
#define MSG_WARN                 LL_WARN, __FILE__, __LINE__, __FUNCTION__
#define MSG_NOTICE               LL_NOTICE, __FILE__, __LINE__, __FUNCTION__
#define MSG_INFO                 LL_INFO, __FILE__, __LINE__, __FUNCTION__
#define MSG_DEBUG                LL_DEBUG, __FILE__, __LINE__, __FUNCTION__

#define MAX_LOG_ENTRY_LENGTH 1024
#define LOG(level, log_id, fmt, args...)  { \
    _log(level, log_id, fmt, ##args);       \
}                                           \

//#define LOG(level, fmt, args...)  { if (level <= LL_ERROR) { fprintf(stderr, fmt, ##args); fprintf(stderr, "\n"); } else { fprintf(stdout, fmt, ##args); fprintf(stdout, "\n"); } }
//#define LOGV(level, fmt, args...) if (level <= LOGGER.log_level()) LOGGER.Write(level, __FILE__, __LINE__, __FUNCTION__, fmt, ##args)L

void _log(int level, int64_t log_id, const char* fmt, ...);

extern void initial_log_id(int seed);
extern int64_t new_log_id();

#endif
