#include "common/log.h"

#include "common/atomic.h"

#include <stdlib.h>
#include <strings.h>

static cceph_atomic64_t g_log_id = { 0 };

void initial_log_id(int32_t prefix) {
    srand((unsigned)time(0));

    int64_t log_id = prefix;
    log_id = (log_id * 10000000000) + rand();

    cceph_atomic_set64(&g_log_id, log_id);
}

int64_t new_log_id() {
   return cceph_atomic_add64(&g_log_id, 1);
}

void _log(int level, int64_t log_id, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[MAX_LOG_ENTRY_LENGTH];
    bzero(buffer, MAX_LOG_ENTRY_LENGTH);

    int offset = sprintf(buffer, "[logid %ld]", log_id);
    offset += vsprintf(buffer + offset, fmt, args);
    offset += sprintf(buffer + offset, "\n");

    FILE* fd = level <= LL_ERROR ? stderr : stdout;
    fprintf(fd, buffer);
    fflush(fd);
    va_end(args);
}
