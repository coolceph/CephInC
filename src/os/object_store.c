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

extern int cceph_os_set_coll_map_key(
        cceph_object_store* os,
        cceph_os_funcs*     os_funcs,
        cceph_os_coll_id_t  cid,
        const char*         key,
        const char*         value,
        int32_t             value_length,
        int64_t             log_id) {

    cceph_rb_root map = CCEPH_RB_ROOT;
    cceph_os_map_node* node = NULL;
    int ret = cceph_os_map_node_new(key, value, value_length, &node, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    ret = cceph_os_map_node_insert(&map, node, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_map_node_free(&node, log_id);
        return ret;
    }

    cceph_os_tran* tran = NULL;
    ret = cceph_os_tran_new(&tran, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_map_node_free(&node, log_id);
        return ret;
    }
    ret = cceph_os_tran_coll_map(tran, cid, &map, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_map_node_free(&node, log_id);
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }
    ret = os_funcs->submit_tran(os, tran, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_map_node_free(&node, log_id);
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }

    cceph_os_map_node_free(&node, log_id);
    cceph_os_tran_free(&tran, log_id);
    return CCEPH_OK;
}

