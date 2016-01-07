#ifndef CCEPH_MESSAGE_H
#define CCEPH_MESSAGE_H

#include "include/types.h"
#include "include/int_types.h"

#include "common/list.h"

#define CCEPH_MSG_OP_UNKNOWN   0
#define CCEPH_MSG_OP_WRITE     1
#define CCEPH_MSG_OP_WRITE_ACK 2
#define CCEPH_MSG_OP_READ      3
#define CCEPH_MSG_OP_READ_ACK  4

typedef struct {
    int8_t  op;
    int64_t log_id;
} msg_header;

#endif
