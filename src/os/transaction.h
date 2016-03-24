#ifndef CCEPH_OS_TRANSACTION_H
#define CCEPH_OS_TRANSACTION_H

#define cceph_os_coll_id_t int32_t

typedef struct {
} cceph_os_transaction;

extern int cceph_os_write(cceph_os_transaction* tran,
        cceph_os_coll_id_t  coll_id,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        const char*         data,
        int64_t             log_id);

#endif
