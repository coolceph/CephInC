#include "message/msg_header.h"

#include <unistd.h>

#include "common/assert.h"
#include "message/io.h"

extern int recv_msg_header(int fd, msg_header* header, int64_t log_id) {
    assert(log_id, header != NULL);

    int ret = recv_int8(fd, &header->op, log_id);
    if (ret != 0) return ret;

    ret = recv_int64(fd, &header->log_id, log_id);
    if (ret != 0) return ret;

    return 0;
}
extern int send_msg_header(int fd, msg_header* header, int64_t log_id) {
    assert(log_id, header != NULL);

    int ret = send_int8(fd, header->op, log_id);
    if (ret != 0) return ret;

    ret = send_int64(fd, header->log_id, log_id);
    if (ret != 0) return ret;

    return 0;
}
extern int free_msg_header(msg_header** header, int64_t log_id) {
    assert(log_id, *header != NULL);

    free(*header);
    *header = NULL;
    return 0;
}

