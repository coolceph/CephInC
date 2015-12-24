#include "common/log.h"

#include "common/atomic.h"

#include <stdlib.h>

static atomic64_t g_log_id = { 0 };

void initial_log_id(int32_t prefix) {
    srand((unsigned)time(0));

    int64_t log_id = prefix;
    log_id = (log_id * 10000000000) + rand();

    atomic_set64(&g_log_id, log_id);
}

int64_t new_log_id() {
   return atomic_add64(&g_log_id, 1);
}
