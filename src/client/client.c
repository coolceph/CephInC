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

#include "message/io.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

int client_write_obj(osdmap* osdmap, int64_t log_id, 
                     char* oid, int64_t offset, int64_t length, char* data) {
    
    msg_write_obj_req req;
    req.header.op = CCEPH_MSG_OP_WRITE;
    req.oid = oid;
    req.offset = offset;
    req.length = length;
    req.data = data;

    int i = 0;
    for (i = 0; i < osdmap->osd_count; i++) {
        char *host = osdmap->osds[i].host;
        int   port = osdmap->osds[i].port;
        
        //TODO: client should send msg by messenger
        /*
         *int ret = send_msg_write_req(host, port, &req, log_id);
         *if (ret != 0) {
         *    LOG(LL_ERROR, log_id, "send write msg to %s:%d error: %d", host, port, ret);
         *    return ret;
         *}
         */

        LOG(LL_INFO, log_id, 
            "send req_write oid: %s, offset: %ld, length: %ld " \
            "to osd %s: %d.", oid, offset, length, host, port);
    }

    return 0;
}
