#include "osd_create.h"

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"

int cceph_osd_create_meta_coll(
        cceph_osd* osd,
        int64_t    log_id) {
    cceph_object_store* os       = osd->os;
    cceph_os_funcs*     os_funcs = osd->os_funcs;

    int8_t is_meta_coll_exist = 0;
    int ret = os_funcs->exist_coll(os, CCEPH_OS_META_COLL_ID, &is_meta_coll_exist, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    } else if (is_meta_coll_exist) {
        return CCEPH_ERR_COLL_ALREADY_EXIST;
    }

    cceph_os_tran* tran = NULL;
    ret = cceph_os_tran_new(&tran, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    ret = cceph_os_coll_create(tran, CCEPH_OS_META_COLL_ID, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }
    ret = os_funcs->submit_tran(os, tran, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }

    return CCEPH_OK;
}

int cceph_osd_create_osd_id(
        cceph_osd* osd,
        int64_t    log_id) {

    cceph_object_store* os       = osd->os;
    cceph_os_funcs*     os_funcs = osd->os_funcs;

    cceph_rb_root      map = CCEPH_RB_ROOT;
    cceph_os_map_node* node_osd_id = NULL;
    cceph_os_tran*     tran = NULL;

    int ret = cceph_os_map_node_new(CCEPH_OS_META_ATTR_OSD_ID,
            (char*)&(osd->osd_id), sizeof(cceph_osd_id_t), &node_osd_id, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    ret = cceph_os_map_node_insert(&map, node_osd_id, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    ret = cceph_os_tran_new(&tran, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    ret = cceph_os_coll_map(tran, CCEPH_OS_META_COLL_ID, &map, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }
    ret = os_funcs->submit_tran(os, tran, log_id);
    if (ret != CCEPH_OK) {
        cceph_os_tran_free(&tran, log_id);
        return ret;
    }

    return CCEPH_OK;
}

int cceph_osd_create(
        cceph_osd* osd,
        int64_t    log_id) {

    assert(log_id, osd != NULL);

    int ret = cceph_osd_create_meta_coll(osd, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "create osd failed: create meta coll failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    cceph_osd_entity osds[3];
    osds[0].id      = 0;
    osds[1].id      = 1;
    osds[2].id      = 2;

    cceph_osdmap osdmap;
    osdmap.epoch     = 0;
    osdmap.pg_count  = 256;
    osdmap.osd_count = 3;
    osdmap.osds      = &osds[0];

    cceph_buffer* buffer = NULL;
    ret = cceph_buffer_new(&buffer, log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    cceph_encode_osdmap(buffer, &osdmap, log_id);

    return CCEPH_OK;
}




