#include "message/msg_write_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"

extern cceph_msg_write_obj_req* cceph_msg_write_obj_new() {
    cceph_msg_write_obj_req* req = malloc(sizeof(cceph_msg_write_obj_req));
    bzero(req, sizeof(cceph_msg_write_obj_req));
    req->header.op = CCEPH_MSG_OP_WRITE;
    return req;
}
extern int cceph_msg_write_obj_req_free(cceph_msg_write_obj_req** req, int64_t log_id) {
    assert(log_id, *req != NULL);

    cceph_msg_write_obj_req* msg = *req;
    if (msg->oid != NULL) {
        free(msg->oid);
    }
    if (msg->data != NULL) {
        free(msg->data);
    }
    free(msg);

    *req = NULL;
    return 0;
}

extern int cceph_msg_write_obj_req_send(int fd, cceph_msg_write_obj_req* req, int64_t log_id) {
    assert(log_id, req != NULL);
    assert(log_id, req->oid != NULL);
    assert(log_id, req->data != NULL);

    int ret = 0;
    CCEPH_SEND_FIELD(client_id, int32, req->client_id);
    CCEPH_SEND_FIELD(req_id, int32, req->req_id);
    CCEPH_SEND_FIELD(oid, string, req->oid);
    CCEPH_SEND_FIELD(offset, int64, req->offset);
    CCEPH_SEND_DATA_FIELD(data, req->length, req->data);
    return 0;
}
extern int cceph_msg_write_obj_req_recv(int fd, cceph_msg_write_obj_req* req, int64_t log_id) {
    assert(log_id, req != NULL);
    assert(log_id, req->oid == NULL);
    assert(log_id, req->data == NULL);

    int ret = 0;
    CCEPH_RECV_FIELD(client_id, int32, &req->client_id);
    CCEPH_RECV_FIELD(req_id, int32, &req->req_id);
    CCEPH_RECV_STRING_FIELD(oid, &req->oid_size, &req->oid);
    CCEPH_RECV_FIELD(offset, int64, &req->offset);
    CCEPH_RECV_DATA_FIELD(data, &req->length, &req->data);
    return ret;
}

extern cceph_msg_write_obj_ack* cceph_msg_write_obj_ack_new() {
    cceph_msg_write_obj_ack* msg = malloc(sizeof(cceph_msg_write_obj_ack));
    bzero(msg, sizeof(cceph_msg_write_obj_ack));
    msg->header.op = CCEPH_MSG_OP_WRITE_ACK;
    return msg;
}
extern int cceph_msg_write_obj_ack_free(cceph_msg_write_obj_ack** msg, int64_t log_id) {
    assert(log_id, *msg != NULL);
    free(*msg); 
    *msg = NULL;
    return 0;
}

extern int cceph_msg_write_obj_ack_recv(int fd, cceph_msg_write_obj_ack* msg, int64_t log_id) {
    assert(log_id, msg != NULL);
    int ret = 0;
    CCEPH_RECV_FIELD(client_id, int32, &msg->client_id);
    CCEPH_RECV_FIELD(req_id, int32, &msg->req_id);
    CCEPH_RECV_FIELD(result, int8, &msg->result);
    return 0;
}
extern int cceph_msg_write_obj_ack_send(int fd, cceph_msg_write_obj_ack* msg, int64_t log_id) {
    assert(log_id, msg != NULL);
    int ret = 0;
    CCEPH_SEND_FIELD(client_id, int32, msg->client_id);
    CCEPH_SEND_FIELD(req_id, int32, msg->req_id);
    CCEPH_SEND_FIELD(result, int8, msg->result);
    return 0;
}
