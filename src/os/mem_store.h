#ifndef CCEPH_MEM_STORE_H
#define CCEPH_MEM_STORE_H

#include "os/object_store.h"

typedef void* cceph_mem_store;

extern cceph_os_funcs* cceph_mem_store_get_funcs();

extern cceph_mem_store* cceph_mem_store_new();

extern int cceph_mem_store_mount(
        cceph_object_store*  os,
        int64_t              log_id);

extern int cceph_mem_store_submit_transaction(
        cceph_object_store*  os,
        cceph_os_transaction transaction,
        int64_t              log_id);

extern int cceph_mem_store_read_object(
        cceph_object_store*  os,
        const char*          oid,
        int64_t              offset,
        int64_t              length,
        char*                data,
        int64_t              log_id);

#endif
