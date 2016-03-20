#include "file_store.h"


extern cceph_file_store* cceph_file_store_new(const char* path) {
    return NULL;
}


extern int cceph_file_store_mount(
        cceph_object_store* os) {
    return 0;
}

extern int cceph_file_store_queue_transaction(
        cceph_object_store* os,
        cceph_os_transaction transaction) {
    return 0;
}

extern int cceph_file_store_read_object(
        cceph_object_store* os,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        char*               data) {
    return 0;
}
