#include "os/transaction.h"

#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/types.h"

int cceph_os_transaction_new(
        cceph_os_transaction** tran,
        int64_t                log_id) {
    assert(log_id,  tran != NULL);
    assert(log_id, *tran == NULL);

    *tran = (cceph_os_transaction*)malloc(sizeof(cceph_os_transaction));
    if (*tran == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*tran)->op_buffer = (cceph_os_transaction_op*)malloc(sizeof(cceph_os_transaction_op) * CCEPH_OS_TRAN_OP_LIST_SIZE);
    if ((*tran)->op_buffer == NULL) {
        free(*tran);
        *tran = NULL;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*tran)->op_buffer_length = CCEPH_OS_TRAN_OP_LIST_SIZE;
    (*tran)->op_buffer_index  = 0;

    return CCEPH_OK;
}

int cceph_os_transactio_check_op_buffer_size(cceph_os_transaction *tran, int64_t log_id) {
    assert(log_id, tran != NULL);
    assert(log_id, tran->op_buffer_index <= tran->op_buffer_length);

    //There is still enough space
    if (tran->op_buffer_index < tran->op_buffer_length) {
        return 0;
    }

    assert(log_id, tran->op_buffer_index == tran->op_buffer_length);

    cceph_os_transaction_op *old_op_buffer = tran->op_buffer;
    int32_t                  old_op_length = tran->op_buffer_length;

    tran->op_buffer_length += CCEPH_OS_TRAN_OP_LIST_SIZE;
    tran->op_buffer = (cceph_os_transaction_op*)malloc(sizeof(cceph_os_transaction_op) * tran->op_buffer_length);
    if (tran->op_buffer == NULL) {
        tran->op_buffer = old_op_buffer;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    memcpy(tran->op_buffer, old_op_buffer, sizeof(cceph_os_transaction_op) * old_op_length);
    return 0;
}

int cceph_os_write(cceph_os_transaction* tran,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        const char*         data,
        int64_t             log_id) {

    assert(log_id, tran != NULL);
    assert(log_id, oid  != NULL);
    assert(log_id, offset >= 0);
    assert(log_id, length > 0);
    assert(log_id, data != NULL);

    int ret = cceph_os_transactio_check_op_buffer_size(tran, log_id);
    if (ret != 0) {
        return ret;
    }

    cceph_os_transaction_op *op = tran->op_buffer + tran->op_buffer_index;
    op->op      = CCEPH_OS_OP_WRITE;
    op->cid     = cid;
    op->oid     = oid;
    op->offset  = offset;
    op->length  = length;
    op->data    = data;
    op->log_id  = log_id;

    tran->op_buffer_index++;
    return 0;
}

int cceph_os_tran_get_op_count(
        cceph_os_transaction* tran,
        int64_t               log_id) {
    assert(log_id, tran != NULL);
    return tran->op_buffer_index;
}

cceph_os_transaction_op* cceph_os_tran_get_op(
        cceph_os_transaction* tran,
        int32_t               index,
        int64_t               log_id) {
    assert(log_id, tran != NULL);

    //Out of range
    if (index >= tran->op_buffer_index) {
        return NULL;
    }

    return (cceph_os_transaction_op*)(tran->op_buffer + index);

}
