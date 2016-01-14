#include "message/msg_write_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/log.h"
#include "message/io.h"

extern int send_msg_write_obj_req(char* host, int port, msg_write_obj_req* req, int64_t log_id) {
/*
 *
 *    ret = send_int8(fd, req->header.op, log_id);
 *    if (ret < 0) return ret;
 *
 *    ret = send_string(fd, req->oid, log_id);
 *    if (ret < 0) return ret;
 *
 *    ret = send_int64(fd, req->offset, log_id);
 *    if (ret < 0) return ret;
 *
 *    ret = send_data(fd, req->length, req->data, log_id);
 *    if (ret < 0) return ret;
 *
 *    close(fd);
 */
    return 0;
}
extern int recv_meg_write_obj_req(int fd, msg_write_obj_req** req_ptr, int64_t log_id) {
    *req_ptr = malloc(sizeof(msg_write_obj_req));
    msg_write_obj_req* req = *req_ptr;

    int ret = recv_int32(fd, &req->client_id, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Recv client_id error from fd %d, errno: %d", fd, ret);
        free(req); *req_ptr = NULL;
        return ret;
    }
    ret = recv_int32(fd, &req->req_id, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Recv req_id error from fd %d, errno: %d", fd, ret);
        free(req); *req_ptr = NULL;
        return ret;
    }
    ret = recv_string(fd, &req->oid_size, &req->oid, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Recv oid_size error from fd %d, errno: %d", fd, ret);
        free(req); *req_ptr = NULL;
        return ret;
    }
    ret = recv_int64(fd, &req->offset, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Recv offset error from fd %d, errno: %d", fd, ret);
        free(req->oid); req->oid = NULL;
        free(req); *req_ptr = NULL;
        return ret;
    }
    ret = recv_data(fd, &req->length, &req->data, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Recv data error from fd %d, errno: %d", fd, ret);
        free(req->oid); req->oid = NULL;
        free(req); *req_ptr = NULL;
        return ret;
    }

    return ret;
}
