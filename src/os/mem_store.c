#include "mem_store.h"

#include <string.h>

#include "common/assert.h"

cceph_os_funcs* cceph_mem_store_get_funcs() {
    cceph_os_funcs *os_funcs = (cceph_os_funcs*)malloc(sizeof(cceph_os_funcs));
    bzero(os_funcs, sizeof(cceph_os_funcs));

    os_funcs->mount                 = cceph_mem_store_mount;
    os_funcs->submit_transaction    = cceph_mem_store_submit_transaction;
    os_funcs->read                  = cceph_mem_store_read_object;

    return os_funcs;
}

cceph_mem_store* cceph_mem_store_new() {
    cceph_mem_store *store = (cceph_mem_store*)malloc(sizeof(cceph_mem_store));
    return store;
}

int cceph_mem_store_mount(
        cceph_object_store* os,
        int64_t             log_id) {
    //MemStore don't need mount
    assert(log_id, os != NULL);
    return 0;
}

int cceph_mem_store_submit_transaction(
        cceph_object_store*  os,
        cceph_os_transaction transaction,
        int64_t              log_id) {
    return 0;
}

int cceph_mem_store_read_object(
        cceph_object_store* os,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        char*               data,
        int64_t             log_id) {
    return 0;
}
