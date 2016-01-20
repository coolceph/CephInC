#include "message/msg_write_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"

extern msg_write_obj_req* malloc_msg_write_obj_req(int64_t log_id) {
    msg_write_obj_req* req = malloc(sizeof(msg_write_obj_req));
    bzero(req, sizeof(msg_write_obj_req));
    return req;
}
extern int free_msg_write_obj_req(msg_write_obj_req** req, int64_t log_id) {
    assert(log_id, *req != NULL);

    msg_write_obj_req* msg = *req;
    if (msg->oid != NULL) {
        free(msg->oid);
    }
    if (msg->data != NULL) {
        free(msg->oid);
    }
    free(msg);

    *req = NULL;
    return 0;
}

extern int send_msg_write_obj_req(int fd, msg_write_obj_req* req, int64_t log_id) {
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
extern int recv_msg_write_obj_req(int fd, msg_write_obj_req* req, int64_t log_id) {
    assert(log_id, req != NULL);
    assert(log_id, req->oid == NULL);
    assert(log_id, req->data == NULL);

    int ret = 0;
    CCEPH_RECV_FIELD(client_id, int32, &req->client_id);
    CCEPH_RECV_FIELD(req_id, int32, &req->req_id);
    CCEPH_RECV_FIELD(offset, int64, &req->offset);
    CCEPH_RECV_STRING_FIELD(oid, &req->oid_size, &req->oid);
    CCEPH_RECV_DATA_FIELD(data, &req->length, &req->data);
    return ret;
}

extern msg_write_obj_ack* malloc_msg_write_obj_ack(int64_t log_id) {
    msg_write_obj_ack* msg = malloc(sizeof(msg_write_obj_ack));
    bzero(msg, sizeof(msg_write_obj_ack));
    return msg;
}
extern int free_msg_write_obj_ack(msg_write_obj_ack** msg, int64_t log_id) {
    assert(log_id, *msg != NULL);
    free(*msg); 
    *msg = NULL;
    return 0;
}

extern int recv_msg_write_obj_ack(int fd, msg_write_obj_ack* msg, int64_t log_id) {
    assert(log_id, msg != NULL);
    int ret = 0;
    CCEPH_RECV_FIELD(client_id, int32, &msg->client_id);
    CCEPH_RECV_FIELD(req_id, int32, &msg->req_id);
    CCEPH_RECV_FIELD(result, int8, &msg->result);
    return 0;
}
extern int send_msg_write_obj_ack(int fd, msg_write_obj_ack* msg, int64_t log_id) {
    assert(log_id, msg != NULL);
    int ret = 0;
    CCEPH_SEND_FIELD(client_id, int32, msg->client_id);
    CCEPH_SEND_FIELD(req_id, int32, msg->req_id);
    CCEPH_RECV_FIELD(result, int8, &msg->result);
    return 0;
}
