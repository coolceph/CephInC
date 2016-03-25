#include "os/transaction.h"

extern cceph_os_transaction* cceph_os_transaction_new() {
    cceph_os_transaction* tran = (cceph_os_transaction*)malloc(cceph_os_transaction);
    tran->ops = (cceph_os_transaction_op*)malloc(
            sizeof(cceph_os_transaction_op) * CCEPH_OS_TRAN_OP_LIST_SIZE);

    return tran;
}

extern int cceph_os_write(cceph_os_transaction* tran,
        cceph_os_coll_id_t  coll_id,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        const char*         data,
        int64_t             log_id) {
}

