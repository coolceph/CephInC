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

    (*node)->cid = cid;
    (*node)->objects = CCEPH_RB_ROOT;

    return CCEPH_OK;
}

extern int cceph_mem_store_coll_node_free(
        cceph_mem_store_coll_node** node,
        int64_t                     log_id) {
    assert(log_id, node != NULL);
    assert(log_id, *node != NULL);

    cceph_mem_store_coll_node   *cnode   = *node;
    cceph_mem_store_object_node *onode   = NULL;
    cceph_rb_node               *rb_node = cceph_rb_first(&cnode->objects);
    while (rb_node) {
        onode = cceph_rb_entry(rb_node, cceph_mem_store_object_node, node);

        cceph_rb_erase(rb_node, &cnode->objects);
        cceph_mem_store_object_node_free(&onode, log_id);

        rb_node = cceph_rb_first(&cnode->objects);
    }

    free(cnode);
    *node = NULL;
    return CCEPH_OK;
}

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

