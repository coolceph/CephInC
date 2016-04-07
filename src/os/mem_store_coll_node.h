#ifndef CCEPH_OS_MEM_STORE_COLL_NODE_H
#define CCEPH_OS_MEM_STORE_COLL_NODE_H

#include "common/rbtree.h"
#include "os/types.h"

typedef struct {
    cceph_os_coll_id_t cid;
    cceph_rb_root objects;

    cceph_rb_node node;
} cceph_mem_store_coll_node;

extern cceph_mem_store_coll_node* cceph_mem_store_coll_node_search(
        cceph_rb_root*     root,
        cceph_os_coll_id_t cid);

extern int cceph_mem_store_coll_node_insert(
        cceph_rb_root             *root,
        cceph_mem_store_coll_node *node);
#endif
