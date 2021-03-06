#include "os/transaction.h"

#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/types.h"

int cceph_os_tran_new(
        cceph_os_tran** tran,
        int64_t         log_id) {
    assert(log_id,  tran != NULL);
    assert(log_id, *tran == NULL);

    *tran = (cceph_os_tran*)malloc(sizeof(cceph_os_tran));
    if (*tran == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*tran)->op_buffer = (cceph_os_tran_op*)malloc(sizeof(cceph_os_tran_op) * CCEPH_OS_TRAN_OP_LIST_SIZE);
    if ((*tran)->op_buffer == NULL) {
        free(*tran);
        *tran = NULL;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*tran)->op_buffer_length = CCEPH_OS_TRAN_OP_LIST_SIZE;
    (*tran)->op_buffer_index  = 0;

    return CCEPH_OK;
}
int cceph_os_tran_free(
        cceph_os_tran** tran,
        int64_t         log_id) {

    assert(log_id,  tran != NULL);
    assert(log_id, *tran != NULL);

    if ((*tran)->op_buffer != NULL) {
        free((*tran)->op_buffer);
        (*tran)->op_buffer = NULL;
    }

    free(*tran);
    *tran = NULL;
    return CCEPH_OK;
}

int cceph_os_tran_get_op_count(
        cceph_os_tran* tran,
        int64_t        log_id) {
    assert(log_id, tran != NULL);
    return tran->op_buffer_index;
}

cceph_os_tran_op* cceph_os_tran_get_op(
        cceph_os_tran* tran,
        int32_t        index,
        int64_t        log_id) {
    assert(log_id, tran != NULL);

    //Out of range
    if (index >= tran->op_buffer_index) {
        return NULL;
    }

    return (cceph_os_tran_op*)(tran->op_buffer + index);
}

int cceph_os_tran_check_op_buffer_size(
        cceph_os_tran* tran,
        int64_t        log_id) {
    assert(log_id, tran != NULL);
    assert(log_id, tran->op_buffer_index <= tran->op_buffer_length);

    //There is still enough space
    if (tran->op_buffer_index < tran->op_buffer_length) {
        return CCEPH_OK;
    }

    assert(log_id, tran->op_buffer_index == tran->op_buffer_length);

    cceph_os_tran_op* old_op_buffer = tran->op_buffer;
    int32_t           old_op_length = tran->op_buffer_length;

    tran->op_buffer_length += CCEPH_OS_TRAN_OP_LIST_SIZE;
    tran->op_buffer = (cceph_os_tran_op*)malloc(sizeof(cceph_os_tran_op) * tran->op_buffer_length);
    if (tran->op_buffer == NULL) {
        tran->op_buffer = old_op_buffer;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    memcpy(tran->op_buffer, old_op_buffer, sizeof(cceph_os_tran_op) * old_op_length);
    return CCEPH_OK;
}

int cceph_os_tran_new_op(
        cceph_os_tran*     tran,
        cceph_os_tran_op** op,
        int64_t            log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, op   != NULL);
    assert(log_id, *op  == NULL);

    int ret = cceph_os_tran_check_op_buffer_size(tran, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    *op = tran->op_buffer + tran->op_buffer_index;
    tran->op_buffer_index++;
    return CCEPH_OK;
}

int cceph_os_tran_coll_create(
        cceph_os_tran*      tran,
        cceph_os_coll_id_t  cid,
        int64_t             log_id) {
    assert(log_id, tran != NULL);
    assert(log_id, cid  >= 0);

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_COLL_CREATE;
    op->cid     = cid;
    op->log_id  = log_id;

    return CCEPH_OK;
}

int cceph_os_tran_coll_remove(
        cceph_os_tran*        tran,
        cceph_os_coll_id_t    cid,
        int64_t               log_id) {
    assert(log_id, tran != NULL);
    assert(log_id, cid  >= 0);

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_COLL_REMOVE;
    op->cid     = cid;
    op->log_id  = log_id;

    return CCEPH_OK;
}

int cceph_os_tran_coll_map(
        cceph_os_tran*      tran,
        cceph_os_coll_id_t  cid,
        cceph_rb_root*      map,
        int64_t             log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, map  != NULL);

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_COLL_MAP;
    op->cid     = cid;
    op->map     = map;
    op->log_id  = log_id;

    return CCEPH_OK;
}

int cceph_os_tran_obj_touch(
        cceph_os_tran*        tran,
        cceph_os_coll_id_t    cid,
        const char*           oid,
        int64_t               log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, oid  != NULL);

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_OBJ_TOUCH;
    op->cid     = cid;
    op->oid     = oid;
    op->log_id  = log_id;

    return CCEPH_OK;
}

int cceph_os_tran_obj_write(
        cceph_os_tran*      tran,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        const char*         data,
        int64_t             log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, oid  != NULL);
    assert(log_id, offset >= 0);
    assert(log_id, length >= 0);
    if (length > 0) {
        assert(log_id, data != NULL);
    }

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_OBJ_WRITE;
    op->cid     = cid;
    op->oid     = oid;
    op->offset  = offset;
    op->length  = length;
    op->data    = data;
    op->log_id  = log_id;

    return CCEPH_OK;
}

int cceph_os_tran_obj_map(
        cceph_os_tran*      tran,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        cceph_rb_root*      map,
        int64_t             log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, oid  != NULL);
    assert(log_id, map  != NULL);

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_OBJ_MAP;
    op->cid     = cid;
    op->oid     = oid;
    op->map     = map;
    op->log_id  = log_id;

    return CCEPH_OK;
}

int cceph_os_tran_obj_remove(
        cceph_os_tran*      tran,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        int64_t             log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, oid  != NULL);

    cceph_os_tran_op *op = NULL;
    int ret = cceph_os_tran_new_op(tran, &op, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    op->op      = CCEPH_OS_OP_OBJ_REMOVE;
    op->cid     = cid;
    op->oid     = oid;
    op->log_id  = log_id;

    return CCEPH_OK;
}

