#ifndef CCEPH_OS_OP_H
#define CCEPH_OS_OP_H

#include "common/types.h"

typedef int32_t cceph_os_op_t;

#define CCEPH_OS_OP_NOOP    0
#define CCEPH_OS_OP_WRITE   1

extern const char* cceph_os_op_to_str(int op);

#endif
