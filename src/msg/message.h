#ifndef CCEPH_MESSAGE_H
#define CCEPH_MESSAGE_H

#define CCEPH_MSG_OP_WRITE 1

struct msg_header {
    int8_t op;
};

//layout:op, oid, offset, length, data
struct msg_write_req {
    struct msg_header header;

    int16_t oid_size;
    char* oid;

    int64_t offset;
    int64_t length;
    char* data;
};

#endif
