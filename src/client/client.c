#include "client/client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "common/log.h"
#include "msg/message.h"

static int send_int8(int fd, int8_t value) {
    int ret = send(fd, &value, sizeof(int8_t), 0);
    if (ret != sizeof(int8_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send int8_t %d error: %d", value, errno);
        return errno;
    }
    return 0;
}
static int send_int64(int fd, int64_t value) {
    int ret = send(fd, &value, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send int64_t %ld error: %d", value, errno);
        return errno;
    }
    return 0;
}
static int send_string(int fd, char* string) {
    int16_t size = strlen(string);
    
    int ret = send(fd, &size, sizeof(int16_t), 0);
    if (ret != sizeof(int16_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send string %s error: %d", string, errno);
        return errno;
    }
    ret = send(fd, string, size, 0);
    if (ret != size) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send string %s error: %d", string, errno);
        return errno;
    }
    return 0;
}
static int send_data(int fd, int64_t length, char* data) {
    int ret = send(fd, &length, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send data error: %d, length: %ld", errno, length);
        return errno;
    }
    ret = send(fd, data, length, 0);
    if (ret != length) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send data error: %d, length: %ld", errno, length);
        return errno;
    }

    return 0;
}

static int send_msg_write_req(char* host, int port, struct msg_write_req* req) {

    struct sockaddr_in server_addr_in;
    bzero(&server_addr_in, sizeof(server_addr_in) );
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr_in.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));

    send_int8(fd, req->header.op);
    send_string(fd, req->oid);
    send_int64(fd, req->offset);
    send_data(fd, req->length, req->data);

    close(fd);
    LOG(LL_INFO, "Send req_write oid: %s, offset: %ld, length: %ld. \n",
                 req->oid, req->offset, req->length);

    return 0;
}

extern int write_obj(struct osdmap* osdmap,
                     char* oid, int64_t offset, int64_t length, char* data) {
    
    struct msg_write_req msg_write_req;
    msg_write_req.header.op = CCEPH_MSG_OP_WRITE;
    msg_write_req.oid = oid;
    msg_write_req.offset = offset;
    msg_write_req.length = length;
    msg_write_req.data = data;

    int i = 0;
    for (i = 0; i < osdmap->osd_count; i++) {
        char *host = osdmap->osds[0].host;
        int  port = osdmap->osds[0].port;
        
        int ret = send_msg_write_req(host, port, &msg_write_req);
        if (ret != 0) {
            LOG(LL_ERROR, "send write msg to %s:%d error: %d", host, port, ret);
            return ret;
        }
    }

    return 0;
}
