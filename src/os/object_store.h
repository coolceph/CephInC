#ifndef CCEPH_OBJECT_STORE_H
#define CCEPH_OBJECT_STORE_H

#include "common/types.h"

#include "os/types.h"
#include "os/transaction.h"

typedef void* cceph_object_store;

typedef int (*cceph_os_mount_func)(
        cceph_object_store* os,
        int64_t             log_id);

typedef int (*cceph_os_submit_transaction_func)(
        cceph_object_store*   os,
        cceph_os_transaction* transaction,
        int64_t               log_id);

//if length <= 0 or length >= object->length, read the whole content
typedef int (*cceph_os_read_object_func)(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        char**              result_data,
        int64_t*            result_length,
        int64_t             log_id);

typedef struct {
    cceph_os_mount_func              mount;
    cceph_os_submit_transaction_func submit_transaction;
    cceph_os_read_object_func        read;
} cceph_os_funcs;

#endif
