#include "osd_create.h"

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"

int cceph_osd_create(
        cceph_osd* osd,
        int64_t    log_id) {

    assert(log_id, osd != NULL);

    cceph_object_store* os       = osd->os;
    cceph_os_funcs*     os_funcs = osd->os_funcs;

    int ret = cceph_os_create_coll(os, os_funcs, CCEPH_OSD_META_COLL_ID, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "create osd failed: create meta coll failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_os_set_coll_map_key(os, os_funcs,
            CCEPH_OSD_META_COLL_ID, CCEPH_OSD_META_ATTR_OSD_ID,
            (char*)&(osd->osd_id), sizeof(cceph_osd_id_t), log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "create osd failed: create osd_id attr failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    cceph_epoch_t max_epoch = 0;
    ret = cceph_os_set_coll_map_key(os, os_funcs,
            CCEPH_OSD_META_COLL_ID, CCEPH_OSD_META_ATTR_OSDMAP_MAX_EPOCH,
            (char*)&(max_epoch), sizeof(cceph_epoch_t), log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "create osd failed: create osdmap_max_epoch attr failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    return CCEPH_OK;

}

