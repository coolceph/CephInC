/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <inttypes.h>

#include "include/errno.h"

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"
#include "message/messenger.h"
#include "message/server_messenger.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

static int do_req_write(msg_write_obj_req* req) {
    int64_t log_id = req->header.log_id;

    LOG(LL_INFO, log_id, "req_write, oid: %s, offset: %lu, length: %lu",
           req->oid, req->offset, req->length);

    char data_dir[] = "./data";
    int max_path_length = 4096;

    char path[max_path_length];
    memset(path, '\0', max_path_length);

    strcat(path, data_dir);
    strcat(path, "/");
    strcat(path, req->oid);

    int oid_fd = open(path, O_RDWR | O_CREAT);
    pwrite(oid_fd, req->data, req->length, req->offset);
    close(oid_fd);

    //TODO: reply to the client
    free(req->oid);
    free(req->data);
    free(req);

    return 0;
}

static int process_message(msg_handle* msg_handle, conn_id_t conn_id, msg_header* message, void* context) {
    int64_t log_id = message->log_id;
    assert(log_id, msg_handle != NULL);
    assert(log_id, message != NULL);
    assert(log_id, context == NULL);

    int8_t op = message->op;
    LOG(LL_NOTICE, log_id, "Porcess message msg from conn %ld, op %d", conn_id, message->op);

    int ret = 0;
    switch (op) {
        case CCEPH_MSG_OP_WRITE:
            ret = do_req_write((msg_write_obj_req*)message);
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


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    int32_t log_prefix = 201;
    initial_log_id(log_prefix);

    int64_t log_id = new_log_id();
    msg_handle* msg_handle = new_msg_handle(&process_message, NULL, log_id);
    server_msg_handle *server_msg_handle = new_server_msg_handle(msg_handle, port, log_id);

    start_server_messenger(server_msg_handle, log_id);

    return 0;
}

