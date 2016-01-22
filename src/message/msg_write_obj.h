#ifndef CCEPH_MESSAGE_MSG_WRITE_OBJ_H
#define CCEPH_MESSAGE_MSG_WRITE_OBJ_H

#include "message/msg_header.h"

typedef struct {
    msg_header header;
    
    int32_t client_id;
    int32_t req_id;

    int16_t oid_size;
    char* oid;

    int64_t offset;
    int64_t length;
    char* data;
} msg_write_obj_req;

extern msg_write_obj_req* malloc_msg_write_obj_req();
extern int free_msg_write_obj_req(msg_write_obj_req** req, int64_t log_id);

extern int recv_msg_write_obj_req(int fd, msg_write_obj_req* req, int64_t log_id);
extern int send_msg_write_obj_req(int fd, msg_write_obj_req* req, int64_t log_id);

#define CCEPH_WRITE_OBJ_ACK_UNKNOWN 0
#define CCEPH_WRITE_OBJ_ACK_OK      1

typedef struct {
    msg_header header;

    int32_t client_id;
    int32_t req_id;

    int8_t result;
} msg_write_obj_ack;

extern msg_write_obj_ack* malloc_msg_write_obj_ack();
extern int free_msg_write_obj_ack(msg_write_obj_ack** msg, int64_t log_id);

extern int recv_msg_write_obj_ack(int fd, msg_write_obj_ack* msg, int64_t log_id);
extern int send_msg_write_obj_ack(int fd, msg_write_obj_ack* msg, int64_t log_id);

#endif
