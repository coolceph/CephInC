#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "os/mem_store_object_node.h"

int cceph_mem_store_object_node_new(
        cceph_mem_store_object_node** node,
        const char*                   oid,
        int64_t                       log_id) {

    assert(log_id, node != NULL);
    assert(log_id, oid != NULL);

    *node = (cceph_mem_store_object_node*)malloc(sizeof(cceph_mem_store_object_node));
    if (*node == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    int oid_length = strlen(oid) + 1;
    (*node)->oid = (char*)malloc(sizeof(char) * oid_length);
    if ((*node)->oid == NULL) {
        free(*node);
        *node = NULL;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }
    bzero((*node)->oid, oid_length);
    strcpy((*node)->oid, oid);

    (*node)->data = NULL;
    (*node)->length = 0;

    return CCEPH_OK;
}

int cceph_mem_store_object_node_free(
        cceph_mem_store_object_node** node,
        int64_t                       log_id) {

    assert(log_id, node != NULL);
    assert(log_id, *node != NULL);

    cceph_mem_store_object_node* onode = *node;
    if (onode->oid != NULL) {
        free(onode->oid);
        onode->oid = NULL;
    }
    if (onode->data != NULL) {
        free(onode->data);
        onode->data = NULL;
    }

    free(onode);
    *node = NULL;

    return CCEPH_OK;
}

int cceph_mem_store_object_node_insert(
        cceph_rb_root               *root,
        cceph_mem_store_object_node *node,
        int64_t                      log_id) {

    assert(log_id, root != NULL);
    assert(log_id, node != NULL);

    cceph_rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        cceph_mem_store_object_node *this = cceph_container_of(*new, cceph_mem_store_object_node, node);
        int result = strcmp(node->oid, this->oid);

        parent = *new;
        if (result < 0) {
            new = &((*new)->rb_left);
        } else if (result > 0) {
            new = &((*new)->rb_right);
        } else {
            return CCEPH_ERR_OBJECT_ALREADY_EXIST;
        }
    }

    /* Add new node and rebalance tree. */
    cceph_rb_link_node(&node->node, parent, new);
    cceph_rb_insert_color(&node->node, root);

    return CCEPH_OK;
}

int cceph_mem_store_object_node_remove(
        cceph_rb_root               *root,
        cceph_mem_store_object_node *node,
        int64_t                      log_id) {

    assert(log_id, root != NULL);
    assert(log_id, node != NULL);

    cceph_rb_erase(&node->node, root);

    return CCEPH_OK;
}

int cceph_mem_store_object_node_search(
        cceph_rb_root*                root,
        const char*                   oid,
        cceph_mem_store_object_node** result,
        int64_t                       log_id) {

    assert(log_id, root != NULL);
    assert(log_id, oid  != NULL);
    assert(log_id, result != NULL);
    assert(log_id, *result == NULL);

    cceph_rb_node *node = root->rb_node;
    while (node) {
        cceph_mem_store_object_node *data = cceph_container_of(node, cceph_mem_store_object_node, node);

        int ret = strcmp(oid, data->oid);
        if (ret < 0) {
            node = node->rb_left;
        } else if (ret > 0) {
            node = node->rb_right;
        } else {
            *result = data;
            return CCEPH_OK;
        }
    }
    return CCEPH_ERR_OBJECT_NOT_EXIST;
}


