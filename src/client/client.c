#include "client/client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"

#include "message/io.h"
#include "message/messenger.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

static int do_object_write_ack(cceph_client *client,
        cceph_messenger* messenger, cceph_conn_id_t conn_id, cceph_msg_write_obj_ack* ack) {

    int64_t log_id = ack->header.log_id;
    assert(log_id, client != NULL);
    assert(log_id, messenger != NULL);
    assert(log_id, conn_id > 0);

    return 0;
}

static int client_process_message(
        cceph_messenger* messenger,
        cceph_conn_id_t conn_id,
        cceph_msg_header* message,
        void* context) {
    //Now we just process msg_write_object_ack
    int64_t log_id = message->log_id;
    assert(log_id, messenger != NULL);
    assert(log_id, message != NULL);
    assert(log_id, context == NULL);

    cceph_client *client = (cceph_client*)context;

    int8_t op = message->op;
    LOG(LL_NOTICE, log_id, "Porcess message msg from conn %ld, op %d", conn_id, message->op);

    int ret = 0;
    switch (op) {
        case CCEPH_MSG_OP_WRITE_ACK:
            ret = do_object_write_ack(client, messenger, conn_id, (cceph_msg_write_obj_ack*)message);
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

extern cceph_client *cceph_client_new(cceph_osdmap* osdmap) {
    int64_t log_id = 0;
    cceph_client *client = (cceph_client*)malloc(sizeof(cceph_client));
    if (client == NULL) {
        LOG(LL_ERROR, log_id, "Failed to malloc cceph_client, maybe not enough memory.");
        return NULL;
    }

    client->messenger = cceph_messenger_new(&client_process_message, client, log_id);
    if (client->messenger == NULL) {
        LOG(LL_ERROR, log_id, "Failed to malloc cceph_client->messenger, maybe not enough memory.");
        free(client);
        client = NULL;
        return NULL;
    }

    client->osdmap = osdmap;
    client->state  = CCEPH_CLIENT_STATE_UNKNOWN;

    return client;
}

extern int cceph_client_init(cceph_client *client) {
    int64_t log_id = cceph_log_new_id();
    LOG(LL_INFO, log_id, "log id for cceph_client_init: %lld.", log_id);

    client->client_id = 0; //TODO: client should get its id from mon
    cceph_atomic_set(&client->req_id, 0);

    cceph_list_head_init(&client->wait_req_list.list_node);
    pthread_mutex_init(&client->wait_req_lock, NULL);
    pthread_cond_init(&client->wait_req_cond, NULL);

    int ret = cceph_messenger_start(client->messenger, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "cceph_messenger_start failed, errno %d.", ret);
    }

    if (ret == 0) {
        client->state = CCEPH_CLIENT_STATE_NORMAL;
    }

    return ret;
}

static int send_req_to_osd(cceph_messenger* msger, cceph_osd_id *osd, cceph_msg_header* req, int64_t log_id) {
    assert(log_id, msger != NULL);
    assert(log_id, osd != NULL);
    assert(log_id, osd->host != NULL);
    assert(log_id, req != NULL);

    const char* host = osd->host;
    const int   port = osd->port;

    cceph_conn_id_t conn_id = cceph_messenger_get_conn(msger, host, port, log_id);
    if (conn_id < 0) {
        LOG(LL_WARN, log_id, "failed to get conn to %s:%d.", host, port);
        return conn_id;
    }
    LOG(LL_INFO, log_id, "get conn_id %d for %s:%d.", conn_id, host, port);

    int ret = cceph_messenger_send_msg(msger, conn_id, (cceph_msg_header*)req, log_id);
    if (ret != 0) {
        LOG(LL_WARN, log_id, "failed to send req to conn %d.", conn_id);
        return ret;
    }

    LOG(LL_INFO, log_id, "send req to conn_id %d success.", conn_id);
    return 0;
}
static int add_req_to_wait_list(cceph_client *client, cceph_msg_header *req, int req_count, int64_t log_id) {
    assert(log_id, client != NULL);
    assert(log_id, req != NULL);

    cceph_client_wait_req *wait_req = (cceph_client_wait_req*)malloc(sizeof(cceph_client_wait_req));
    wait_req->req = req;
    wait_req->req_count = req_count;
    wait_req->ack_count = 0;
    wait_req->commit_count = 0;

    pthread_mutex_lock(&client->wait_req_lock);
    cceph_list_add(&wait_req->list_node, &client->wait_req_list.list_node);
    pthread_mutex_unlock(&client->wait_req_lock);

    return 0;
}
static int wait_for_req(cceph_client *client, cceph_msg_header *req, int64_t log_id) {
    assert(log_id, client != NULL);
    assert(log_id, req != NULL);

    return 0;
}
extern int cceph_client_write_obj(cceph_client* client,
                     char* oid, int64_t offset, int64_t length, char* data) {
    int64_t log_id = cceph_log_new_id();

    assert(log_id, client != NULL);
    assert(log_id, client->state == CCEPH_CLIENT_STATE_NORMAL);
    assert(log_id, oid != NULL);
    assert(log_id, data != NULL);

    cceph_msg_write_obj_req *req = cceph_msg_write_obj_req_new();
    req->header.op = CCEPH_MSG_OP_WRITE;
    req->header.log_id = log_id;
    req->client_id = client->client_id;
    req->req_id = cceph_atomic_add(&client->req_id, 1);
    req->oid = oid;
    req->offset = offset;
    req->length = length;
    req->data = data;

    cceph_messenger *msger = client->messenger;
    cceph_osdmap    *osdmap = client->osdmap;
    int failed_count = 0, success_count = 0, i = 0;
    for (i = 0; i < osdmap->osd_count; i++) {
        int ret = send_req_to_osd(msger, &osdmap->osds[i], (cceph_msg_header*)req, log_id);
        if (ret == 0) {
            success_count++;
        } else {
            failed_count++;
        }
    }

    //TODO: this should be a opinion of the poll
    if (success_count <= osdmap->osd_count / 2) {
        LOG(LL_ERROR, log_id, "Can't send req to enough servers, success %d, failed %d. ",
                success_count, failed_count);
        return CCEPH_ERR_NOT_ENOUGH_SERVER;
    }

    LOG(LL_DEBUG, log_id, "Send req success, success %d, failed %d.",
            success_count, failed_count);

    int ret = add_req_to_wait_list(client, (cceph_msg_header*)req, osdmap->osd_count, log_id);
    assert(log_id, ret == 0);
    LOG(LL_DEBUG, log_id, "Add req to wait list.");

    ret = wait_for_req(client, (cceph_msg_header*)req, log_id);
    LOG(LL_INFO, log_id, "req finished, ret %d.", ret);

    return ret;
}

int TEST_add_req_to_wait_list(cceph_client *client,
        cceph_msg_header *req, int req_count, int64_t log_id) {
    return add_req_to_wait_list(client, req, req_count, log_id);
}
