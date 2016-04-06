#include "os/op.h"

const char* cceph_os_op_to_str(int op) {
    switch(op) {
        case CCEPH_OS_OP_NOOP:
            return "noop";
        case CCEPH_OS_OP_WRITE:
            return "write";
        default:
            return "unkown_op";
    }

}
