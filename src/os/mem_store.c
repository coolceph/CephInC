#include "mem_store.h"


extern cceph_mem_store* cceph_mem_store_new() {
    cceph_mem_store *store = (cceph_mem_store*)malloc(sizeof(cceph_mem_store));
    return store;
}


extern int cceph_mem_store_mount(
        cceph_object_store* os,
        int64_t log_id) {
    assert(log_id, os != NULL);
    return 0;
}

extern int cceph_mem_store_submit_transaction(
        cceph_object_store* os,
        cceph_os_transaction transaction) {
    return 0;
}

extern int cceph_mem_store_read_object(
        cceph_object_store* os,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        char*               data) {
    return 0;
}
