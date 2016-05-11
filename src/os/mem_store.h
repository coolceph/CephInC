#ifndef CCEPH_MEM_STORE_H
#define CCEPH_MEM_STORE_H

#include <pthread.h>

#include "common/rbtree.h"
#include "os/object_store.h"
#include "os/types.h"
#include "os/mem_store_coll_node.h"
#include "os/mem_store_object_node.h"

typedef struct {
    cceph_rb_root colls;
    pthread_mutex_t lock;
} cceph_mem_store;

extern cceph_os_funcs* cceph_mem_store_get_funcs();

extern int cceph_mem_store_new(
        cceph_mem_store** store,
        int64_t log_id);

extern int cceph_mem_store_mount(
        cceph_object_store*  os,
        int64_t              log_id);

extern int cceph_mem_store_submit_tran(
        cceph_object_store*  os,
        cceph_os_tran*       tran,
        int64_t              log_id);

extern int cceph_mem_store_read_obj(
        cceph_object_store*  os,
        cceph_os_coll_id_t   cid,
        const char*          oid,
        int64_t              offset,
        int64_t              length,
        int64_t*             result_length,
        char**               result_data,
        int64_t              log_id);

#endif
