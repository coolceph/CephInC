#ifndef CCEPH_OS_TRANSACTION_H
#define CCEPH_OS_TRANSACTION_H

#include "common/types.h"
#include "os/types.h"

#define CCEPH_OS_TRAN_OP_LIST_SIZE 8

typedef struct {
    int64_t             log_id;
    cceph_os_op_t       op;
    cceph_os_coll_id_t  cid;
    const char*         oid;
    int64_t             offset;
    int64_t             length;
    const char*         data;
} cceph_os_transaction_op;

typedef struct {
    cceph_os_transaction_op *op_buffer;
    int32_t                  op_buffer_length;
    int32_t                  op_buffer_index;
} cceph_os_transaction;

extern int cceph_os_transaction_new(
        cceph_os_transaction** tran,
        int64_t                log_id);

extern int cceph_os_transaction_free(
        cceph_os_transaction** tran,
        int64_t                log_id);

//if object already existed, do nothing
extern int cceph_os_touch(
        cceph_os_transaction* tran,
        cceph_os_coll_id_t    cid,
        const char*           oid,
        int64_t               log_id);

//if object not exists, create it
//length == 0 means touch object
extern int cceph_os_write(
        cceph_os_transaction* tran,
        cceph_os_coll_id_t    cid,
        const char*           oid,
        int64_t               offset,
        int64_t               length,
        const char*           data,
        int64_t               log_id);

//if object not existed, return CCEPH_ERR_OBJECT_NOT_EXISTED
extern int cceph_os_remove(
        cceph_os_transaction* tran,
        cceph_os_coll_id_t    cid,
        const char*           oid,
        int64_t               log_id);

//if coll existed, do nothing
extern int cceph_os_create_coll(
        cceph_os_transaction* tran,
        cceph_os_coll_id_t    cid,
        int64_t               log_id);

//if coll not existed, return CCEPH_ERR_COLL_NOT_EXISTED
extern int cceph_os_remove_coll(
        cceph_os_transaction* tran,
        cceph_os_coll_id_t    cid,
        int64_t               log_id);

extern int cceph_os_tran_get_op_count(
        cceph_os_transaction* tran,
        int64_t               log_id);

extern cceph_os_transaction_op* cceph_os_tran_get_op(
        cceph_os_transaction* tran,
        int32_t               index,
        int64_t               log_id);

#endif
