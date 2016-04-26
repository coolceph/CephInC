#ifndef CCEPH_OS_TYPES_H
#define CCEPH_OS_TYPES_H

#include "common/types.h"
#include "common/rbtree.h"

#define CCEPH_OS_OP_NOOP          0
#define CCEPH_OS_OP_WRITE         1
#define CCEPH_OS_OP_TOUCH         2
#define CCEPH_OS_OP_REMOVE        3
#define CCEPH_OS_OP_CREATE_COLL   4
#define CCEPH_OS_OP_REMOVE_COLL   5

typedef int32_t cceph_os_coll_id_t;
typedef int32_t cceph_os_op_t;

typedef struct {
    char*   key; //key should be a string

    char*   value;
    int32_t value_length;

    cceph_rb_node node;
} cceph_os_omap_node;

extern const char* cceph_os_op_to_str(int op);

extern int cceph_os_coll_id_cmp(cceph_os_coll_id_t cid_a, cceph_os_coll_id_t cid_b);

#endif
