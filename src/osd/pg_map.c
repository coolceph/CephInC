#include "osd/pg_map.h"

#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/util.h"

int cceph_pg_map_insert(
        cceph_rb_root *root,
        cceph_pg      *node,
        int64_t       log_id) {

    assert(log_id, root != NULL);
    assert(log_id, node != NULL);

    cceph_rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        cceph_pg *this = cceph_container_of(*new, cceph_pg, node);
        int result = intcmp(node->pg_id, this->pg_id);

        parent = *new;
        if (result < 0) {
            new = &((*new)->rb_left);
        } else if (result > 0) {
            new = &((*new)->rb_right);
        } else {
            return CCEPH_ERR_MAP_NODE_ALREADY_EXIST;
        }
    }

    /* Add new node and rebalance tree. */
    cceph_rb_link_node(&node->node, parent, new);
    cceph_rb_insert_color(&node->node, root);

    return CCEPH_OK;
}

int cceph_pg_map_remove(
        cceph_rb_root *root,
        cceph_pg      *node,
        int64_t       log_id) {

    assert(log_id, root != NULL);
    assert(log_id, node != NULL);

    cceph_rb_erase(&node->node, root);

    return CCEPH_OK;
}

int cceph_pg_map_search(
        cceph_rb_root *root,
        cceph_pg_id_t  pg_id,
        cceph_pg       **result,
        int64_t        log_id) {

    assert(log_id, root != NULL);
    assert(log_id, result != NULL);
    assert(log_id, *result == NULL);

    cceph_rb_node *node = root->rb_node;

    while (node) {
        cceph_pg *data = cceph_container_of(node, cceph_pg, node);

        int ret = intcmp(pg_id, data->pg_id);
        if (ret < 0) {
            node = node->rb_left;
        } else if (ret > 0) {
            node = node->rb_right;
        } else {
            *result = data;
            return CCEPH_OK;
        }
    }
    return CCEPH_ERR_MAP_NODE_NOT_EXIST;
}
