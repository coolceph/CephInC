#ifndef CCEPH_MSG_WRITE_OBJ_H
#define CCEPH_MSG_WRITE_OBJ_H

#include "msg/msg_header.h"

struct msg_write_obj_req {
    struct msg_header header;
    
    int32_t client_id;
    int32_t req_id;

    int16_t oid_size;
    char* oid;

    int64_t offset;
    int64_t length;
    char* data;
};

extern int send_msg_write_req(char* host, int port, struct msg_write_obj_req* req, int64_t log_id);

#endif
