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

typedef int32_t cceph_os_op_t;
typedef int32_t cceph_os_coll_id_t;

extern const char* cceph_os_op_to_str(int op);

extern int cceph_os_coll_id_cmp(cceph_os_coll_id_t cid_a, cceph_os_coll_id_t cid_b);

typedef struct {
    char*   key;            //key should be a string

    char*   value;
    int32_t value_length;

    cceph_rb_node node;
} cceph_os_map_node;

extern int cceph_os_map_node_new(
        const char*         key,
        const char*         value,
        int                 value_length,
        cceph_os_map_node** node,
        int64_t             log_id);

extern int cceph_os_map_node_free(
        cceph_os_map_node** node,
        int64_t             log_id);

extern int cceph_os_map_node_insert(
        cceph_rb_root       *root,
        cceph_os_map_node   *node,
        int64_t             log_id);

extern int cceph_os_map_node_remove(
        cceph_rb_root       *root,
        cceph_os_map_node   *node,
        int64_t             log_id);

extern int cceph_os_map_node_search(
        cceph_rb_root*      root,
        const char*         oid,
        cceph_os_map_node** result,
        int64_t             log_id);

//This method will use the input_tree to update the result_tree
//The update rule is:
//  for each key in input_tree
//      1) if the key is not in result_tree and its input_value is not NULL:
//         add it to result_tree
//      2) if the key is in the result_tree and its input_value is not NULL:
//         update the result_tree to new value
//      3) if the key is in the result_tree and its input_value is NULL:
//         remove the key from result_tree
//      4) if the key is not in result_tree and its input_value is NULL:
//         do nothing
extern int cceph_os_map_update(cceph_rb_root* result_tree, cceph_rb_root* input_tree, int64_t log_id);

#endif
