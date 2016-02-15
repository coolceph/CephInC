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
#include "message/messenger.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

static int client_process_message(msg_handle* msg_handle, conn_id_t conn_id, msg_header* message, void* context) {
    return 0;
}

extern client_handle *new_client_handle(osdmap* osdmap) {
    int64_t log_id = 0;
    client_handle *handle = (client_handle*)malloc(sizeof(client_handle));
    if (handle == NULL) {
        LOG(LL_ERROR, log_id, "Failed to malloc client_handle, maybe not enough memory.");
        return NULL;
    }

    handle->msg_handle = new_msg_handle(&client_process_message, handle, log_id);
    if (handle->msg_handle == NULL) {
        LOG(LL_ERROR, log_id, "Failed to malloc client_handle->msg_handle, maybe not enough memory.");
        free(handle);
        handle = NULL;
        return NULL;
    }

    handle->osdmap = osdmap;
    handle->state  = CCEPH_CLIENT_STATE_UNKNOWN;

    return handle;
}

extern int init_client(client_handle *handle) {
    return 0;
}

extern int client_write_obj(osdmap* osdmap, int64_t log_id,
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
