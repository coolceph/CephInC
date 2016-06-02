#include "common/buffer.h"

#include "common/assert.h"
#include "common/errno.h"
#include "common/types.h"

#define CCEPH_BUFFER_NODE_LENGTH 4096

int cceph_buffer_node_new(
        cceph_buffer_node** node_ptr,
        int64_t             log_id) {

    assert(log_id, node_ptr  != NULL);
    assert(log_id, *node_ptr == NULL);

    cceph_buffer_node* node = (cceph_buffer_node*)malloc(sizeof(cceph_buffer_node));
    if (node == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    node->data = (char*)malloc(sizeof(char) * CCEPH_BUFFER_NODE_LENGTH);
    if (node->data == NULL) {
        free(node);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    node->ptr    = node->data;
    node->length = CCEPH_BUFFER_NODE_LENGTH;
    node->next   = NULL;

    *node_ptr = node;
    return CCEPH_OK;
}

int cceph_buffer_node_free(
        cceph_buffer_node** node,
        int64_t             log_id) {
    assert(log_id, node  != NULL);
    assert(log_id, *node != NULL);

    free((*node)->data);
    free(*node);
    *node = NULL;

    return CCEPH_OK;
}

int cceph_buffer_new(
        cceph_buffer** buffer_ptr,
        int64_t        log_id) {

    assert(log_id, buffer_ptr  != NULL);
    assert(log_id, *buffer_ptr == NULL);

    cceph_buffer_node* node = NULL;
    int ret = cceph_buffer_node_new(&node, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    cceph_buffer* buffer = (cceph_buffer*)malloc(sizeof(cceph_buffer));
    if (buffer == NULL) {
        cceph_buffer_node_free(&node, log_id);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    buffer->head    = node;
    buffer->current = node;
    buffer->length  = node->length;
    buffer->offset  = 0;

    return CCEPH_OK;
}

int cceph_buffer_free(
        cceph_buffer** buffer_ptr,
        int64_t        log_id) {

    assert(log_id, buffer_ptr  != NULL);
    assert(log_id, *buffer_ptr != NULL);

    cceph_buffer* buffer = *buffer_ptr;
    cceph_buffer_node* node = buffer->head;
    cceph_buffer_node* next = buffer->head;
    while (node != NULL) {
        next = node->next;
        cceph_buffer_node_free(&node, log_id);
        node = next;
    }

    free(buffer);
    *buffer_ptr = NULL;

    return CCEPH_OK;
}
