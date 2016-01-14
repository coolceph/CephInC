#include "message/msg_write_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"

extern msg_write_obj_req* malloc_msg_write_obj_req() {
    //TODO: impl & test
    return NULL;
}
extern int free_msg_write_obj_req(msg_write_obj_req** req) {
    //TODO: impl & test
    return 0;
}

extern int send_msg_write_obj_req(int fd, msg_write_obj_req* req, int64_t log_id) {
    //TODO: impl & test
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
extern int recv_meg_write_obj_req(int fd, msg_write_obj_req* req, int64_t log_id) {
    //TODO: impl & test
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
