#include "common/log.h"

#include "common/atomic.h"

#include <math.h>

static atomic64_t g_log_id = { 0 };

void initial_log_id(int prefix) {
}

int64_t new_log_id() {
   return atomic_add64(&g_log_id, 1);
}
