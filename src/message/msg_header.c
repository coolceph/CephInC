#include "message/msg_header.h"

extern int msg_header_read(int fd, msg_header* header, int64_t log_id) {
    //TODO: mv to other file
    //int8_t op;
    //if(read_int8(fd, &op, log_id) != 0) return NULL;

    //msg_write_obj_req* msg = malloc(sizeof(msg_write_obj_req));
    //msg->header.op = op;

    //if(read_string(fd, &(msg->oid_size), &(msg->oid), log_id) != 0) return NULL;
    //if(read_int64(fd, &(msg->offset), log_id) != 0) return NULL;
    //if(read_data(fd, &(msg->length), &(msg->data), log_id) != 0) return NULL;
    
    return 0;
}
