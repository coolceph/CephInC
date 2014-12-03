#ifndef CCEPH_GLOBAL_H
#define CCEPH_GLOBAL_H

#include "common/context.h"
#include "common/config.h"

void init_global();

extern struct cceph_context *g_context;
extern struct cceph_config *g_conf;

#endif