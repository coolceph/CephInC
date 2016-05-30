#include "osd/osd.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"
#include "common/option.h"

#include "message/msg_write_obj.h"

int io_write_object(const char* oid,
        int64_t offset, int64_t length,
        const char* data,
        int64_t log_id) {

    //Get the path of obj
    //TODO: Now we just use data dir for all objects
    char data_dir[] = "./data";
    int max_path_length = 4096;

    char path[max_path_length];
    memset(path, '\0', max_path_length);
    strcat(path, data_dir);
    strcat(path, "/");
    strcat(path, oid);

    int oid_fd = open(path, O_RDWR | O_CREAT);
    if (oid_fd < 0) {
        LOG(LL_ERROR, log_id, "Open fd for oid %s failed, path %s, errno %d", oid, path, oid_fd);
        return oid_fd;
    }

    int ret = pwrite(oid_fd, data, length, offset);
    if (ret != length) {
        LOG(LL_ERROR, log_id, "Write oid %s failed, path %s, errno %d", oid, path, ret);
        close(oid_fd); //We don't ensure close success, just try to release the fd
        return ret;
    }

    ret = close(oid_fd);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Close fd %d for oid %s failed, path %s, errno %d", oid_fd, oid, path, ret);
    }

    return ret;
}

//TODO: The write process is not so simply by our design, this is just a demo
//TODO: We should define/impl the object_store/transaction first before impl the write process
//TODO: Aslo the client behavier should be consisent with the osd
int do_object_write_req(cceph_messenger* messenger, cceph_conn_id_t conn_id, cceph_msg_write_obj_req* req) {
    int64_t log_id = req->header.log_id;

    int32_t client_id = req->client_id;
    int64_t req_id    = req->req_id;
    char*   oid       = req->oid;
    int64_t length    = req->length;
    int64_t offset    = req->offset;
    char* data        = req->data;

    LOG(LL_INFO, log_id, "do_req_write, client_id %d, req_id %ld, oid %s, offset %lu, length %lu.",
            client_id, req_id, oid, offset, length);

    int ret = io_write_object(oid, offset, length, data, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "io_write_object failed for req_id %ld, errno %d(%s).", req_id, ret, cceph_errno_str(ret));
    } else {
        LOG(LL_INFO, log_id, "io_write_object success for req_id %ld.", req_id);
    }

    int result = ret == 0 ? CCEPH_WRITE_OBJ_ACK_OK : ret; //TODO: we need more CCEPH_WRITE_OBJ_ACK_*

    cceph_msg_write_obj_ack* ack = cceph_msg_write_obj_ack_new();
    assert(log_id, ack != NULL);

    ack->header.op = CCEPH_MSG_OP_WRITE_ACK;
    ack->header.log_id = log_id;
    ack->client_id = client_id;
    ack->req_id = req_id;
    ack->result = result;

    LOG(LL_INFO, log_id, "cceph_msg_write_obj_ack_send to client %d, req_id %d, result %d.", client_id, req_id, result);
    ret = cceph_messenger_send_msg(messenger, conn_id, (cceph_msg_header*)ack, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "cceph_msg_write_obj_ack_send failed for client %d, req_id %d, errno %d(%s).",
                client_id, req_id, ret, cceph_errno_str(ret));
    } else {
        LOG(LL_INFO, log_id, "cceph_msg_write_obj_ack_send success for client %d, req_id %d.",
                client_id, req_id);
    }

    int ret2 = cceph_msg_write_obj_req_free(&req, log_id);
    assert(log_id, ret2 == 0);
    ret2 = cceph_msg_write_obj_ack_free(&ack, log_id);
    assert(log_id, ret2 == 0);

    return ret;
}

int cceph_osd_initial(
        cceph_osd**         osd,
        cceph_osd_id_t      osd_id,
        cceph_object_store* os,
        cceph_os_funcs*     os_funcs,
        int64_t             log_id) {

    assert(log_id, osd != NULL);
    assert(log_id, *osd == NULL);
    assert(log_id, osd_id >= 0);
    assert(log_id, os != NULL);
    assert(log_id, os_funcs != NULL);

    *osd = (cceph_osd*)malloc(sizeof(cceph_osd));
    if (*osd == NULL) {
        LOG(LL_ERROR, log_id, "Initial osd error, no enough memory");
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }
    cceph_messenger* msger = cceph_messenger_new(
            &cceph_osd_process_message,
            *osd,
            g_cceph_option.osd_msg_workthread_count,
            log_id);
    if (msger == NULL) {
        free(*osd);
        LOG(LL_ERROR, log_id, "Initial osd error, no enough memory");
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    cceph_server_messenger *smsger = new_cceph_server_messenger(
            msger,
            g_cceph_option.osd_port_base + osd_id,
            log_id);
    if (smsger == NULL) {
        free(*osd);
        cceph_messenger_free(&msger, log_id);
        LOG(LL_ERROR, log_id, "Initial osd error, no enough memory");
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    cceph_osd* osd_ptr = *osd;
    osd_ptr->osd_id    = osd_id;
    osd_ptr->os        = os;
    osd_ptr->os_funcs  = os_funcs;
    osd_ptr->msger     = msger;
    osd_ptr->smsger    = smsger;

    return CCEPH_OK;
}
extern int cceph_osd_start(
        cceph_osd* osd,
        int64_t    log_id) {
    assert(log_id, osd != NULL);

    return cceph_server_messenger_start(osd->smsger, log_id);
}

int cceph_osd_process_message(
        cceph_messenger* messenger,
        cceph_conn_id_t conn_id,
        cceph_msg_header* message,
        void* context) {
    int64_t log_id = message->log_id;
    assert(log_id, messenger != NULL);
    assert(log_id, message != NULL);
    assert(log_id, context == NULL);

    int8_t op = message->op;
    LOG(LL_NOTICE, log_id, "Porcess message msg from conn %ld, op %d.", conn_id, message->op);

    int ret = 0;
    switch (op) {
        case CCEPH_MSG_OP_WRITE:
            ret = do_object_write_req(messenger, conn_id, (cceph_msg_write_obj_req*)message);
            break;
        default:
            ret = CCEPH_ERR_UNKNOWN_OP;
    }

    if (ret == 0) {
        LOG(LL_NOTICE, log_id, "Porcess message msg from conn %ld, op %d success.",
                conn_id, op);
    } else {
        LOG(LL_INFO, log_id, "Porcess message msg from conn %ld, op %d failed, errno %d(%s).",
                conn_id, op, ret, cceph_errno_str(ret));
    }

    return ret;
}
