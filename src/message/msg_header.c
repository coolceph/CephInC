#include "message/msg_header.h"

extern int recv_msg_header(int fd, msg_header* header, int64_t log_id) {
    //TODO: mv to other file
    //int8_t op;
    //if(read_int8(fd, &op, log_id) != 0) return NULL;
    return 0;
}
extern int send_msg_header(int fd, msg_header* header, int64_t log_id) {
    return 0;
}
extern int free_msg_header(msg_header** header) {
    return 0;
}

