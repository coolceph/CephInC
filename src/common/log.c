#include "common/log.h"

#include "common/atomic.h"

#include <stdlib.h>
#include <strings.h>

cceph_atomic64_t cceph_g_log_id = { 0 };

void cceph_log_initial_id(int32_t prefix) {
    srand((unsigned)time(0));

    int64_t log_id = prefix;
    log_id = (log_id * 10000000000) + rand();

    cceph_atomic_set64(&cceph_g_log_id, log_id);
}

int64_t cceph_log_new_id() {
   return cceph_atomic_add64(&cceph_g_log_id, 1);
}

void _cceph_log(int level, int64_t log_id, const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[MAX_LOG_ENTRY_LENGTH];
    bzero(buffer, MAX_LOG_ENTRY_LENGTH);

    int offset = sprintf(buffer, "[logid %ld]", log_id);
    offset += sprintf(buffer + offset, "[%s:%d]", file, line);
    offset += vsprintf(buffer + offset, fmt, args);
    offset += sprintf(buffer + offset, "\n");

    FILE* fd = level <= LL_ERROR ? stderr : stdout;
    fprintf(fd, buffer);
    fflush(fd);
    va_end(args);
}
