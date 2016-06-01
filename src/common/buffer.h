#ifndef CCEPH_BUFFER_H
#define CCEPH_BUFFER_H

typedef struct cceph_buffer_node_ cceph_buffer_node_;
struct cceph_buffer_node_ {
    char*   data;
    char*   ptr;
    int32_t length;

    cceph_buffer_node_ *next;
};
typedef cceph_buffer_node_ cceph_buffer_node;

typedef struct {
    cceph_buffer_node* head;
    cceph_buffer_node* current;

    int32_t data_length;
} cceph_buffer;


#endif
