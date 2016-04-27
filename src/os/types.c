#include "os/types.h"

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
        case CCEPH_OS_OP_CREATE_COLL:
            return "CollectionCreate";
        case CCEPH_OS_OP_REMOVE_COLL:
            return "CollectionRemove";
        case CCEPH_OS_OP_TOUCH:
            return "ObjectTouch";
        case CCEPH_OS_OP_WRITE:
            return "ObjectWrite";
        case CCEPH_OS_OP_MAP:
            return "ObjectMap";
        case CCEPH_OS_OP_REMOVE:
            return "ObjectRemove";
        default:
            return "UnknownOp";
    }

}


