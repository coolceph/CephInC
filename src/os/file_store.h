#ifndef CCEPH_FILE_STORE_H
#define CCEPH_FILE_STORE_H

#include "os/object_store.h"

typedef struct {
} cceph_file_store;

extern cceph_file_store* cceph_file_store_new(const char* path);


extern int cceph_file_store_mount(
        cceph_object_store* os);

extern int cceph_file_store_queue_transaction(
        cceph_object_store* os,
        cceph_os_transaction transaction);

extern int cceph_file_store_read_object(
        cceph_object_store* os,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        char*               data);

#endif
