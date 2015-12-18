#ifndef CCEPH_MESSAGE_H
#define CCEPH_MESSAGE_H

#define CCEPH_MSG_OP_UNKNOWN   00
#define CCEPH_MSG_OP_WRITE     01
#define CCEPH_MSG_OP_WRITE_ACK 02
#define CCEPH_MSG_OP_READ      03
#define CCEPH_MSG_OP_READ_ACK  04

struct msg_header {
    int8_t op;
};

#endif
