#include "client/client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "include/errno.h"

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"
#include "message/messenger.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

static int do_object_write_ack(client_handle *handle,
        msg_handle* msg_handle, conn_id_t conn_id, msg_write_obj_ack* ack) {

    return 0;
}

static int client_process_message(msg_handle* msg_handle, conn_id_t conn_id, msg_header* message, void* context) {
    //Now we just process msg_write_object_ack
    int64_t log_id = message->log_id;
    assert(log_id, msg_handle != NULL);
    assert(log_id, message != NULL);
    assert(log_id, context == NULL);

    client_handle *handle = (client_handle*)context;

    int8_t op = message->op;
    LOG(LL_NOTICE, log_id, "Porcess message msg from conn %ld, op %d", conn_id, message->op);

    int ret = 0;
    switch (op) {
        case CCEPH_MSG_OP_WRITE_ACK:
            ret = do_object_write_ack(handle, msg_handle, conn_id, (msg_write_obj_ack*)message);
            break;
        default:
            ret = CCEPH_ERR_UNKNOWN_OP;
    }

    if (ret == 0) {
        LOG(LL_NOTICE, log_id, "Porcess message msg from conn %ld, op %d success.", conn_id, op);
    } else {
        LOG(LL_INFO, log_id, "Porcess message msg from conn %ld, op %d failed, errno %d", conn_id, op, ret);
    }

    return ret;
}

extern client_handle *cceph_new_client_handle(osdmap* osdmap) {
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

extern int cceph_initial_client(client_handle *handle) {
    int64_t log_id = new_log_id();
    LOG(LL_INFO, log_id, "log id for cceph_initial_client: %lld.", log_id);

    init_list_head(&handle->wait_req_list.list_node);
    pthread_mutex_init(&handle->wait_req_lock, NULL);
    pthread_cond_init(&handle->wait_req_cond, NULL);

    int ret = start_messager(handle->msg_handle, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "start_messager failed, errno %d.", ret);
    }

    if (ret == 0) {
        handle->state = CCEPH_CLIENT_STATE_NORMAL;
    }

    return ret;
}

extern int cceph_client_write_obj(osdmap* osdmap, int64_t log_id,
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
