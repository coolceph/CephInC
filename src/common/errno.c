#include "errno.h"

const char* cceph_errno_str(int err_no) {
    switch (err_no) {
        case CCEPH_OK:
            return "OK";
        case CCEPH_ERR_CONN_NOT_FOUND:
            return "ConnectionNotFound";
        case CCEPH_ERR_CONN_CLOSED:
            return "ConnectionClosed";
        case CCEPH_ERR_WRITE_CONN_ERR:
            return "WriteConnectionError";
        case CCEPH_ERR_UNKNOWN_OP:
            return "UnknownOp";
        case CCEPH_ERR_NOT_ENOUGH_SERVER:
            return "NotEnoughServer";
        case CCEPH_ERR_WRONG_CLIENT_ID:
            return "WrongClientID";
    }
    return "UnknowError";
}


