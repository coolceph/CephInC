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
#include "common/network.h"
#include "msg/message.h"
#include "msg/msg_write_obj.h"


static int send_msg_write_req(char* host, int port, struct msg_write_obj_req* req) {

    struct sockaddr_in server_addr_in;
    bzero(&server_addr_in, sizeof(server_addr_in) );
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr_in.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(fd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));
    if (ret < 0) {
        LOG(LL_ERROR, "connect to %s:%d error: %d", host, port, ret);
        return ret;
    }

    ret = send_int8(fd, req->header.op);
    if (ret < 0) return ret;

    ret = send_string(fd, req->oid);
    if (ret < 0) return ret;

    ret = send_int64(fd, req->offset);
    if (ret < 0) return ret;

    ret = send_data(fd, req->length, req->data);
    if (ret < 0) return ret;

    close(fd);
    return 0;
}

int client_write_obj(struct osdmap* osdmap,
                     char* oid, int64_t offset, int64_t length, char* data) {
    
    struct msg_write_obj_req req;
    req.header.op = CCEPH_MSG_OP_WRITE;
    req.oid = oid;
    req.offset = offset;
    req.length = length;
    req.data = data;

    int i = 0;
    for (i = 0; i < osdmap->osd_count; i++) {
        char *host = osdmap->osds[i].host;
        int   port = osdmap->osds[i].port;
        
        int ret = send_msg_write_req(host, port, &req);
        if (ret != 0) {
            LOG(LL_ERROR, "send write msg to %s:%d error: %d", host, port, ret);
            return ret;
        }

        LOG(LL_INFO, "send req_write oid: %s, offset: %ld, length: %ld " \
                     "to osd %s: %d.", oid, offset, length, host, port);
    }

    return 0;
}
