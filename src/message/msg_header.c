#include "message/msg_header.h"

#include <unistd.h>

#include "common/assert.h"
#include "message/io.h"

extern msg_header* malloc_msg_header(int64_t log_id) {
    //TODO: test
    msg_header* msg = malloc(sizeof(msg_header));
    assert(log_id, msg != NULL);

    msg->op = CCEPH_MSG_OP_UNKNOWN;
    msg->log_id = 0;
    return msg;
}
extern int free_msg_header(msg_header** header, int64_t log_id) {
    assert(log_id, *header != NULL);

    free(*header);
    *header = NULL;
    return 0;
}

extern int recv_msg_header(int fd, msg_header* header, int64_t log_id) {
    assert(log_id, header != NULL);

    int ret = 0;
    CCEPH_RECV_FIELD(op, int8, &header->op);
    CCEPH_RECV_FIELD(log_id, int64, &header->log_id);
    return 0;
}
extern int send_msg_header(int fd, msg_header* header, int64_t log_id) {
    assert(log_id, header != NULL);

    int ret = 0;
    CCEPH_SEND_FIELD(op, int8, header->op);
    CCEPH_SEND_FIELD(log_id, int64, header->log_id);
    return 0;
}
