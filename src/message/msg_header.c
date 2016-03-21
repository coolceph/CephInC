#include "message/msg_header.h"

#include <unistd.h>

#include "common/assert.h"
#include "message/io.h"

const char* cceph_str_msg_op(int op) {
    switch (op) {
        case CCEPH_MSG_OP_WRITE:
            return "Write";
        case CCEPH_MSG_OP_WRITE_ACK:
            return "WriteAck";
        case CCEPH_MSG_OP_READ:
            return "Read";
        case CCEPH_MSG_OP_READ_ACK:
            return "ReadAck";
        default:
            return "Unknown";
    }
}

cceph_msg_header* cceph_msg_header_new(int64_t log_id) {
    cceph_msg_header* msg = malloc(sizeof(cceph_msg_header));
    assert(log_id, msg != NULL);

    msg->op = CCEPH_MSG_OP_UNKNOWN;
    msg->log_id = 0;
    return msg;
}
int cceph_msg_header_free(cceph_msg_header** header, int64_t log_id) {
    assert(log_id, *header != NULL);

    free(*header);
    *header = NULL;
    return 0;
}

int cceph_msg_header_recv(int fd, cceph_msg_header* header, int64_t log_id) {
    assert(log_id, header != NULL);

    int ret = 0;
    CCEPH_RECV_FIELD(op, int8, &header->op);
    CCEPH_RECV_FIELD(log_id, int64, &header->log_id);
    return 0;
}
int cceph_msg_header_send(int fd, cceph_msg_header* header, int64_t log_id) {
    assert(log_id, header != NULL);

    int ret = 0;
    CCEPH_SEND_FIELD(op, int8, header->op);
    CCEPH_SEND_FIELD(log_id, int64, header->log_id);
    return 0;
}
