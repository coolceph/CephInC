#include "os/op.h"

const char* cceph_os_op_to_str(int op) {
    switch(op) {
        case CCEPH_OS_OP_NOOP:
            return "NoOp";
        case CCEPH_OS_OP_WRITE:
            return "WriteObject";
        case CCEPH_OS_OP_TOUCH:
            return "TouchObject";
        case CCEPH_OS_OP_REMOVE:
            return "RemoveObject";
        case CCEPH_OS_OP_CREATE_COLL:
            return "CreateCollection";
        case CCEPH_OS_OP_REMOVE_COLL:
            return "RemoveCollection";
        default:
            return "UnknownOp";
    }

}
