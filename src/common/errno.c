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
        case CCEPH_ERR_NO_ENOUGH_MEM:
            return "NoEnoughMemory";
        case CCEPH_ERR_COLL_ALREADY_EXIST:
            return "CollectionAlreadyExist";
        case CCEPH_ERR_COLL_NOT_EXIST:
            return "CollectionNotExist";
        case CCEPH_ERR_OBJECT_ALREADY_EXIST:
            return "ObjectAlreadyExist";
        case CCEPH_ERR_OBJECT_NOT_EXIST:
            return "ObjectNotExist";
        case CCEPH_ERR_MAP_NODE_ALREADY_EXIST:
            return "MapNodeAlreadyExist";
        case  CCEPH_ERR_MAP_NODE_NOT_EXIST:
            return "MapNodeNotExist";
        case CCEPH_ERR_UNKNOWN_OS_OP:
            return "UnknownOsOp";
    }
    return "UnknowError";
}


