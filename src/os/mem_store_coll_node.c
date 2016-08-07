#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/rbtree.h"
#include "os/mem_store_coll_node.h"
#include "os/mem_store_object_node.h"

extern int cceph_mem_store_coll_node_new(
        cceph_os_coll_id_t          cid,
        cceph_mem_store_coll_node** node,
        int64_t                     log_id) {
    assert(log_id, node != NULL);
    assert(log_id, *node == NULL);
    assert(log_id, cid >= 0);

    *node = (cceph_mem_store_coll_node*)malloc(sizeof(cceph_mem_store_coll_node));
    if (*node == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*node)->cid     = cid;
    (*node)->objects = CCEPH_RB_ROOT;
    (*node)->map     = CCEPH_RB_ROOT;

    return CCEPH_OK;
}

extern int cceph_mem_store_coll_node_free(
        cceph_mem_store_coll_node** node,
        int64_t                     log_id) {
    assert(log_id, node != NULL);
    assert(log_id, *node != NULL);

    cceph_mem_store_coll_node* cnode = *node;

    int ret = cceph_mem_store_object_node_free_tree(&cnode->objects, log_id);
    assert(log_id, ret == CCEPH_OK);

    ret = cceph_os_map_node_free_tree(&cnode->map, log_id);
    assert(log_id, ret == CCEPH_OK);

    free(cnode);
    *node = NULL;
    return CCEPH_OK;
}

CCEPH_IMPL_MAP(mem_store_coll_node, cceph_os_coll_id_t, cid, cceph_os_coll_id_cmp);
