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
#include "common/atomic.h"
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

    if (ack->client_id != client->client_id) {
        LOG(LL_ERROR, log_id, "client %d reviced a write obj ack which not belong to it but client %d.",
                client->client_id, ack->client_id);
        return CCEPH_ERR_WRONG_CLIENT_ID;
    }

    //Add the corresponseding wait_req->ack_count
    struct cceph_list_head *pos;
    cceph_client_wait_req *wait_req = NULL;
    cceph_msg_write_obj_req *write_req = NULL;
    pthread_mutex_lock(&client->wait_req_lock);
    cceph_list_for_each(pos, &(client->wait_req_list.list_node)) {
        wait_req = cceph_list_entry(pos, cceph_client_wait_req, list_node);
        if (wait_req->req->op != CCEPH_MSG_OP_WRITE) {
            continue;
        }

        assert(log_id, write_req == NULL);
        write_req = (cceph_msg_write_obj_req*)wait_req->req;
        if (write_req->req_id != ack->req_id) {
            write_req = NULL;
            continue;
        }

        //TODO: from which osd?
        wait_req->ack_count++;

        LOG(LL_INFO, log_id, "req %ld has receive a ack. req_count %d, ack_count %d, commit_count %d",
                write_req->req_id, wait_req->req_count, wait_req->ack_count, wait_req->commit_count);

        break;
    }

    if (write_req == NULL) {
        LOG(LL_INFO, log_id, "req %ld is not found from wait_list, maybe already finished, "
                "the req is from osd.?.", ack->req_id); //TODO: need osd.id here
    } else if (wait_req->ack_count > wait_req->req_count / 2) {
        //TODO: it should be a option of pool
        LOG(LL_INFO, log_id, "req %ld has receive enough ack, it is finished.", ack->req_id);
        pthread_cond_signal(&client->wait_req_cond);
    }

    pthread_mutex_unlock(&client->wait_req_lock);

    return CCEPH_OK;
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
    assert(log_id, context != NULL);

    cceph_client *client = (cceph_client*)context;

    int8_t op = message->op;
    LOG(LL_NOTICE, log_id, "Process message msg from conn %ld, op %d", conn_id, message->op);

    int ret = 0;
    switch (op) {
        case CCEPH_MSG_OP_WRITE_ACK:
            ret = do_object_write_ack(client, messenger, conn_id, (cceph_msg_write_obj_ack*)message);
            break;
        default:
            ret = CCEPH_ERR_UNKNOWN_OP;
    }

    if (ret == 0) {
        LOG(LL_NOTICE, log_id, "Process message msg from conn %ld, op %d success.", conn_id, op);
    } else {
        LOG(LL_INFO, log_id, "Process message msg from conn %ld, op %d failed, errno %d", conn_id, op, ret);
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
	client->messenger->context = client;

    client->osdmap = osdmap;
    client->state  = CCEPH_CLIENT_STATE_UNKNOWN;

    return client;
}

extern int cceph_client_init(cceph_client *client) {
    int64_t log_id = cceph_log_new_id();
    LOG(LL_INFO, log_id, "log id for cceph_client_init: %lld.", log_id);

    client->client_id = 0; //TODO: client should get its id from mon
    cceph_atomic_set64(&client->req_id, 0);

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
//If finished, remove the wait_req from the list and return it, else return NULL;
//Caller must hold client->wait_req_lock
static cceph_client_wait_req *is_req_finished(cceph_client *client, cceph_msg_header *req, int64_t log_id) {
    assert(log_id, client != NULL);
    assert(log_id, req != NULL);

    struct cceph_list_head *pos;
    cceph_client_wait_req *wait_req = NULL;
    cceph_client_wait_req *result = NULL;
    cceph_list_for_each(pos, &(client->wait_req_list.list_node)) {
        wait_req = cceph_list_entry(pos, cceph_client_wait_req, list_node);
        if (wait_req->req != req) {
            continue;
        }

        if (wait_req->commit_count > 0) {
            result = wait_req;
            break;
        }

        //TODO: This should be an option of poll
        if (wait_req->ack_count > wait_req->req_count / 2) {
            result = wait_req;
            break;
        }
    }
    if (result != NULL) {
        cceph_list_delete(&result->list_node);
    }

    return result;
}
static int wait_for_req(cceph_client *client, cceph_msg_header *req, int64_t log_id) {
    assert(log_id, client != NULL);
    assert(log_id, req != NULL);

    cceph_client_wait_req* wait_req = NULL;
    while (wait_req == NULL) {
        pthread_mutex_lock(&client->wait_req_lock);
        wait_req = is_req_finished(client, req, log_id);
        if (wait_req != NULL) {
            pthread_mutex_unlock(&client->wait_req_lock);
            break;
        }

        pthread_cond_wait(&client->wait_req_cond, &client->wait_req_lock);
        wait_req = is_req_finished(client, req, log_id);
        pthread_mutex_unlock(&client->wait_req_lock);
    }

    LOG(LL_INFO, log_id, "req finished, req_count %d, ack_count %d, commit_count %d.",
            wait_req->req_count, wait_req->ack_count, wait_req->commit_count);

    free(wait_req);
    return 0;
}
extern int cceph_client_write_obj(cceph_client* client,
                     char* oid, int64_t offset, int64_t length, char* data) {
    int64_t log_id = cceph_log_new_id();

    assert(log_id, client != NULL);
    assert(log_id, client->state == CCEPH_CLIENT_STATE_NORMAL);
    assert(log_id, oid != NULL);
    assert(log_id, data != NULL);

    cceph_messenger *msger = client->messenger;
    cceph_osdmap    *osdmap = client->osdmap;

    cceph_msg_write_obj_req *req = cceph_msg_write_obj_req_new();
    req->header.op = CCEPH_MSG_OP_WRITE;
    req->header.log_id = log_id;
    req->client_id = client->client_id;
    req->req_id = cceph_atomic_add64(&client->req_id, 1);
    req->oid = oid;
    req->offset = offset;
    req->length = length;
    req->data = data;

    int ret = add_req_to_wait_list(client, (cceph_msg_header*)req, osdmap->osd_count, log_id);
    assert(log_id, ret == 0);
    LOG(LL_DEBUG, log_id, "Add req to wait list.");

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

        //TODO: remove it from wait_req_list;

        return CCEPH_ERR_NOT_ENOUGH_SERVER;
    }

    LOG(LL_DEBUG, log_id, "Send req success, success %d, failed %d.",
            success_count, failed_count);


    ret = wait_for_req(client, (cceph_msg_header*)req, log_id);
    LOG(LL_INFO, log_id, "req %ld finished, ret %d.", req->req_id, ret);

    return ret;
}

int TEST_cceph_client_add_req_to_wait_list(cceph_client *client,
        cceph_msg_header *req, int req_count, int64_t log_id) {
    return add_req_to_wait_list(client, req, req_count, log_id);
}
int TEST_cceph_client_send_req_to_osd(cceph_messenger* msger,
        cceph_osd_id *osd, cceph_msg_header* req, int64_t log_id) {
    return send_req_to_osd(msger, osd, req, log_id);
}
int TEST_cceph_client_do_object_write_ack(cceph_client *client,
        cceph_messenger* messenger, cceph_conn_id_t conn_id, cceph_msg_write_obj_ack* ack) {
    return do_object_write_ack(client, messenger, conn_id, ack);
}
int TEST_cceph_client_process_message(
        cceph_messenger* messenger,
        cceph_conn_id_t conn_id,
        cceph_msg_header* message,
        void* context) {
    return client_process_message(messenger, conn_id, message, context);
}
