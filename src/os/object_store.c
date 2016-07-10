#include "object_store.h"

#include "common/errno.h"

int cceph_os_create_coll(
        cceph_object_store* os,
        cceph_os_funcs*     os_funcs,
        cceph_os_coll_id_t  cid,
        int64_t             log_id) {

    cceph_os_tran* tran = NULL;
    int ret = cceph_os_tran_new(&tran, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    ret = cceph_os_tran_coll_create(tran, cid, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }
    ret = os_funcs->submit_tran(os, tran, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }

    cceph_os_tran_free(&tran, log_id);
    return CCEPH_OK;
}

int cceph_os_remove_coll(
        cceph_object_store* os,
        cceph_os_funcs*     os_funcs,
        cceph_os_coll_id_t  cid,
        int64_t             log_id) {

    cceph_os_tran* tran = NULL;
    int ret = cceph_os_tran_new(&tran, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    ret = cceph_os_tran_coll_remove(tran, cid, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }
    ret = os_funcs->submit_tran(os, tran, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }

    cceph_os_tran_free(&tran, log_id);
    return CCEPH_OK;
}

