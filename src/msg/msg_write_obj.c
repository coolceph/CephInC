#include "msg/msg_write_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/log.h"
#include "common/network.h"

int send_msg_write_req(char* host, int port, struct msg_write_obj_req* req, int64_t log_id) {

    struct sockaddr_in server_addr_in;
    bzero(&server_addr_in, sizeof(server_addr_in) );
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr_in.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(fd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));
    if (ret < 0) {
        LOG(LL_ERROR, log_id, "connect to %s:%d error: %d", host, port, ret);
        return ret;
    }

    ret = send_int8(fd, req->header.op, log_id);
    if (ret < 0) return ret;

    ret = send_string(fd, req->oid, log_id);
    if (ret < 0) return ret;

    ret = send_int64(fd, req->offset, log_id);
    if (ret < 0) return ret;

    ret = send_data(fd, req->length, req->data, log_id);
    if (ret < 0) return ret;

    close(fd);
    return 0;
}
