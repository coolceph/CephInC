#ifndef CCEPH_MESSAGE_MSG_WRITE_OBJ_H
#define CCEPH_MESSAGE_MSG_WRITE_OBJ_H

#include "message/msg_header.h"

typedef struct {
    cceph_msg_header header;

    int32_t client_id; //src client
    int64_t req_id;    //by client

    int16_t oid_size;
    char*   oid;

    int64_t offset;
    int64_t length;
    char* data;
} cceph_msg_write_obj_req;

extern cceph_msg_write_obj_req* cceph_msg_write_obj_req_new();
extern int cceph_msg_write_obj_req_free(cceph_msg_write_obj_req** req, int64_t log_id);
extern int cceph_msg_write_obj_req_recv(int fd, cceph_msg_write_obj_req* req, int64_t log_id);
extern int cceph_msg_write_obj_req_send(int fd, cceph_msg_write_obj_req* req, int64_t log_id);

#define CCEPH_WRITE_OBJ_ACK_UNKNOWN 0
#define CCEPH_WRITE_OBJ_ACK_OK      1

typedef struct {
    cceph_msg_header header;

    int32_t client_id;
    int64_t req_id;

    int8_t result;
} cceph_msg_write_obj_ack;

extern cceph_msg_write_obj_ack* cceph_msg_write_obj_ack_new();
extern int cceph_msg_write_obj_ack_free(cceph_msg_write_obj_ack** msg, int64_t log_id);
extern int cceph_msg_write_obj_ack_recv(int fd, cceph_msg_write_obj_ack* msg, int64_t log_id);
extern int cceph_msg_write_obj_ack_send(int fd, cceph_msg_write_obj_ack* msg, int64_t log_id);

#endif
