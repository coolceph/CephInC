#include "common/buffer.h"

#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/types.h"

int cceph_buffer_node_new(
        cceph_buffer_node** node_ptr,
        int64_t             log_id) {

    assert(log_id, node_ptr  != NULL);
    assert(log_id, *node_ptr == NULL);

    *node_ptr = (cceph_buffer_node*)malloc(sizeof(cceph_buffer_node));
    if (*node_ptr == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*node_ptr)->ptr    = (*node_ptr)->data;
    (*node_ptr)->length = CCEPH_BUFFER_NODE_LENGTH;
    (*node_ptr)->next   = NULL;

    return CCEPH_OK;
}

int cceph_buffer_node_free(
        cceph_buffer_node** node,
        int64_t             log_id) {
    assert(log_id, node  != NULL);
    assert(log_id, *node != NULL);

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

    *buffer_ptr = (cceph_buffer*)malloc(sizeof(cceph_buffer));
    if (*buffer_ptr == NULL) {
        cceph_buffer_node_free(&node, log_id);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*buffer_ptr)->head    = node;
    (*buffer_ptr)->current = node;
    (*buffer_ptr)->length  = node->length;
    (*buffer_ptr)->offset  = 0;

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

int cceph_buffer_append(
        cceph_buffer* buffer,
        char*         data,
        int32_t       length,
        int64_t       log_id) {

    assert(log_id, buffer != NULL);
    assert(log_id, data   != NULL);
    assert(log_id, length >  0);

    while (length > 0) {
        cceph_buffer_node* current_node = buffer->current;
        int32_t current_node_offset = current_node->ptr - current_node->data;
        int32_t node_empty_size = current_node->length - current_node_offset;
        if (node_empty_size == 0) {
            cceph_buffer_node* new_node = NULL;
            int ret = cceph_buffer_node_new(&new_node, log_id);
            if (ret != CCEPH_OK) {
                return ret;
            }

            buffer->current->next =  new_node;
            buffer->current       =  new_node;
            buffer->length        += new_node->length;

            current_node          =  new_node;
            node_empty_size       =  current_node->length;
        }

        int32_t copy_size = MIN(length, node_empty_size);
        memcpy(current_node->ptr, data, copy_size);

        data   += copy_size;
        length -= copy_size;

        current_node->ptr += copy_size;
        buffer->offset    += copy_size;
        assert(log_id, current_node->ptr <= current_node->data + current_node->length);
        assert(log_id, buffer->offset <= buffer->length);
    }

    return CCEPH_OK;
}
