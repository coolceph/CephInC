#ifndef CCEPH_BUFFER_H
#define CCEPH_BUFFER_H

#include "common/types.h"

typedef struct cceph_buffer_node_ cceph_buffer_node_;
struct cceph_buffer_node_ {
    char*   data;
    char*   ptr;     //Point to current write position
    int32_t length;

    cceph_buffer_node_ *next;
};
typedef cceph_buffer_node_ cceph_buffer_node;

typedef struct {
    cceph_buffer_node* head;
    cceph_buffer_node* current;

    int32_t length;
    int32_t offset;
} cceph_buffer;

extern int cceph_buffer_new(
        cceph_buffer** buffer,
        int64_t        log_id);


#endif
