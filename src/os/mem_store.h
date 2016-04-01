#ifndef CCEPH_MEM_STORE_H
#define CCEPH_MEM_STORE_H

#include "os/object_store.h"

#include "common/rbtree.h"

typedef struct {
    cceph_rb_root colls;
} cceph_mem_store;

typedef struct {
    char* oid;
    char* data;
    int64_t length;

    cceph_rb_node node;
} cceph_mem_store_object_node;

typedef struct {
    cceph_os_coll_id_t cid;
    cceph_rb_root objects;

    cceph_rb_node node;
} cceph_mem_store_coll_node;

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

extern cceph_mem_store_coll_node* TEST_cceph_mem_store_coll_node_search(
        cceph_rb_root*     root,
        cceph_os_coll_id_t cid);

extern int TEST_cceph_mem_store_coll_node_insert(
        cceph_rb_root *root,
        cceph_mem_store_coll_node *node);

#endif
