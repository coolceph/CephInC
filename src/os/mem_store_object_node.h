#ifndef CCEPH_MEM_STORE_OBJECT_NODE_H
#define CCEPH_MEM_STORE_OBJECT_NODE_H

#include "common/rbtree.h"
#include "os/types.h"

typedef struct {
    char*         oid;
    char*         data;
    int64_t       length;
    cceph_rb_root map;

    cceph_rb_node node;
} cceph_mem_store_object_node;

extern int cceph_mem_store_object_node_new(
        const char*                   oid,
        cceph_mem_store_object_node** node,
        int64_t                       log_id);

extern int cceph_mem_store_object_node_free(
        cceph_mem_store_object_node** node,
        int64_t                       log_id);

extern int cceph_mem_store_object_node_free_tree(
        cceph_rb_root*                tree,
        int64_t                       log_id);

CCEPH_DEFINE_MAP(mem_store_object_node, const char*, oid);

#endif
