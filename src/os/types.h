#ifndef CCEPH_OS_TYPES_H
#define CCEPH_OS_TYPES_H

#include "common/types.h"
#include "common/rbtree.h"

#define CCEPH_OS_OP_NOOP          0
#define CCEPH_OS_OP_COLL_CREATE   1
#define CCEPH_OS_OP_COLL_REMOVE   2
#define CCEPH_OS_OP_COLL_MAP      3
#define CCEPH_OS_OP_OBJ_TOUCH     4
#define CCEPH_OS_OP_OBJ_WRITE     5
#define CCEPH_OS_OP_OBJ_MAP       6
#define CCEPH_OS_OP_OBJ_REMOVE    7

typedef int32_t cceph_os_coll_id_t;
typedef int32_t cceph_os_op_t;

typedef struct {
    char*   key; //key should be a string

    char*   value;
    int32_t value_length;

    cceph_rb_node node;
} cceph_os_map_node;

extern const char* cceph_os_op_to_str(int op);

extern int cceph_os_coll_id_cmp(cceph_os_coll_id_t cid_a, cceph_os_coll_id_t cid_b);

#endif
