#ifndef CCEPH_MEM_STORE_OBJECT_NODE_H
#define CCEPH_MEM_STORE_OBJECT_NODE_H

#include "common/rbtree.h"
#include "os/types.h"

typedef struct {
    char* oid;
    char* data;
    int64_t length;

    cceph_rb_node node;
} cceph_mem_store_object_node;

extern int cceph_mem_store_object_node_new(
        cceph_mem_store_object_node** node,
        const char*                   oid,
        int64_t                       log_id);

extern int cceph_mem_store_object_node_free(
        cceph_mem_store_object_node** node,
        int64_t                       log_id);

extern int cceph_mem_store_object_node_insert(
        cceph_rb_root               *root,
        cceph_mem_store_object_node *node,
        int64_t                      log_id);

extern int cceph_mem_store_object_node_remove(
        cceph_rb_root               *root,
        cceph_mem_store_object_node *node,
        int64_t                      log_id);

extern int cceph_mem_store_object_node_search(
        cceph_rb_root*                root,
        const char*                   oid,
        cceph_mem_store_object_node** result,
        int64_t                       log_id);

#endif
