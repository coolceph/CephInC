#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "os/mem_store_coll_node.h"

int cceph_mem_store_coll_node_search(
        cceph_rb_root*              root,
        cceph_os_coll_id_t          cid,
        cceph_mem_store_coll_node** result,
        int64_t                     log_id) {

    assert(log_id, root != NULL);
    assert(log_id, cid >=0);
    assert(log_id, result != NULL);
    assert(log_id, *result == NULL);

    cceph_rb_node *node = root->rb_node;

    while (node) {
        cceph_mem_store_coll_node *data = cceph_container_of(node, cceph_mem_store_coll_node, node);

        int ret = cceph_os_coll_id_cmp(cid, data->cid);
        if (ret < 0) {
            node = node->rb_left;
        } else if (ret > 0) {
            node = node->rb_right;
        } else {
            *result = data;
            return CCEPH_OK;
        }
    }
    return CCEPH_ERR_COLL_NOT_EXIST;
}

int cceph_mem_store_coll_node_insert(
        cceph_rb_root             *root,
        cceph_mem_store_coll_node *node,
        int64_t                    log_id) {

    assert(log_id, root != NULL);
    assert(log_id, node != NULL);

    cceph_rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        cceph_mem_store_coll_node *this = cceph_container_of(*new, cceph_mem_store_coll_node, node);
        int result = cceph_os_coll_id_cmp(node->cid, this->cid);

        parent = *new;
        if (result < 0) {
            new = &((*new)->rb_left);
        } else if (result > 0) {
            new = &((*new)->rb_right);
        } else {
            return CCEPH_ERR_COLL_ALREADY_EXIST;
        }
    }

    /* Add new node and rebalance tree. */
    cceph_rb_link_node(&node->node, parent, new);
    cceph_rb_insert_color(&node->node, root);

    return CCEPH_OK;
}

