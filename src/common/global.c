#include "common/global.h"

#include <stddef.h>

struct cceph_context *g_context = NULL;
struct cceph_config  *g_conf    = NULL;

void cceph_init_global() {
  //we will init g_context
  //and read config from /etc/cceph/cceph.conf
}
