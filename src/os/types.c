#include "os/types.h"

#include "common/assert.h"
#include "common/errno.h"
#include "common/rbtree.h"

extern int cceph_os_coll_id_cmp(cceph_os_coll_id_t cid_a, cceph_os_coll_id_t cid_b) {
    if (cid_a > cid_b) {
        return 1;
    } else if (cid_a < cid_b) {
        return -1;
    } else {
        return 0;
    }
}

const char* cceph_os_op_to_str(int op) {
    switch(op) {
        case CCEPH_OS_OP_NOOP:
            return "NoOp";
        case CCEPH_OS_OP_COLL_CREATE:
            return "CollectionCreate";
        case CCEPH_OS_OP_COLL_REMOVE:
            return "CollectionRemove";
        case CCEPH_OS_OP_OBJ_TOUCH:
            return "ObjectTouch";
        case CCEPH_OS_OP_OBJ_WRITE:
            return "ObjectWrite";
        case CCEPH_OS_OP_OBJ_MAP:
            return "ObjectMap";
        case CCEPH_OS_OP_OBJ_REMOVE:
            return "ObjectRemove";
        default:
            return "UnknownOp";
    }

}

int cceph_os_map_update(cceph_rb_root* result_tree, cceph_rb_root* input_tree, int64_t log_id) {
    assert(log_id, result_tree != NULL);
    assert(log_id, input_tree  != NULL);

    return CCEPH_OK;
}

