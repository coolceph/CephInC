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

const char* cceph_str_msg_op(int op);

typedef struct {
    int8_t  op;
    int64_t log_id;
} cceph_msg_header;

extern cceph_msg_header* cceph_msg_header_new(int64_t log_id);
extern int cceph_msg_header_free(cceph_msg_header** header, int64_t log_id);

extern int cceph_msg_header_recv(int fd, cceph_msg_header* header, int64_t log_id);
extern int cceph_msg_header_send(int fd, cceph_msg_header* header, int64_t log_id);

#endif
