#ifndef CCEPH_MSG_WRITE_OBJ_H
#define CCEPH_MSG_WRITE_OBJ_H

#include "msg/message.h"

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

#endif
