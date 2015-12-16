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

static int send_string(int fd, char* string) {
    int16_t size = strlen(string);
    
    int ret = send(fd, &size, sizeof(int16_t), 0);
    if (ret < 0) {
        return ret;
    }
    ret = send(fd, string, size, 0);
    if (ret < 0) {
        return ret;
    }
}

static int send_msg_write_req(char* host, int port, struct msg_write_req* req) {

    char* oid = req->oid;

    struct sockaddr_in server_addr_in;
    bzero(&server_addr_in, sizeof(server_addr_in) );
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr_in.sin_addr);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(cfd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));

    int8_t op = 1;
    send(cfd, &op, sizeof(int8_t), 0);

    int16_t oid_size = strlen(oid);
    //send(cfd, &oid_size, sizeof(int16_t), 0);
    //send(cfd, oid, oid_size, 0);

    //send(cfd, &offset, sizeof(int64_t), 0);

    //send(cfd, &length, sizeof(int64_t), 0);
    //send(cfd, data, length, 0);

    close(cfd);
    //printf("Send req_write oid: %s, offset: %ld, length: %ld. \n",
    //        oid, offset, length);

    return 0;
}

extern int write_obj(struct osdmap* osdmap,
                     char* oid, int64_t offset, int64_t length, char* data) {
    
    struct msg_write_req msg_write_req;
    msg_write_req.oid = oid;
    msg_write_req.offset = offset;
    msg_write_req.length = length;
    msg_write_req.data = data;

    for (int i = 0; i < osdmap->osd_count; i++) {
        char *host = osdmap->osds[0].host;
        int  port = osdmap->osds[0].port;
        
        int ret = send_write_msg(host, port, &msg_write_req);
        if (ret != 0) {
            LOG(LL_ERROR, "send write msg to %s:%d error: %d", host, port, ret);
            return ret;
        }
    }

    return 0;
}
