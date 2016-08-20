#include "osd/osd.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"
#include "common/option.h"
#include "common/osdmap.h"

#include "message/msg_write_obj.h"

#include "osd/osd_msg_process.h"

int cceph_osd_initial(
        cceph_osd**         osd,
        cceph_osd_id_t      osd_id,
        cceph_object_store* os,
        cceph_os_funcs*     os_funcs,
        int64_t             log_id) {

    assert(log_id, osd != NULL);
    assert(log_id, *osd == NULL);
    assert(log_id, osd_id >= 0);
    assert(log_id, os != NULL);
    assert(log_id, os_funcs != NULL);

    *osd = (cceph_osd*)malloc(sizeof(cceph_osd));
    if (*osd == NULL) {
        LOG(LL_ERROR, log_id, "Initial osd error, no enough memory");
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    cceph_messenger* msger = NULL;
    int ret = cceph_messenger_new(
            &msger,
            &cceph_osd_process_message,
            *osd,
            g_cceph_option.osd_msg_workthread_count,
            log_id);
    if (ret != CCEPH_OK) {
        free(*osd);
        LOG(LL_ERROR, log_id, "Initial osd error, new messenger failed, errno %d(%s).",
                ret, cceph_errno_str(ret));
        return ret;
    }

    cceph_server_messenger *smsger = NULL;
    ret = cceph_server_messenger_new(
            &smsger,
            msger,
            g_cceph_option.osd_port_base + osd_id,
            log_id);
    if (ret != CCEPH_OK) {
        free(*osd);
        cceph_messenger_free(&msger, log_id);
        LOG(LL_ERROR, log_id, "Initial osd error, new server messenger failed, errno %d(%s).",
                ret, cceph_errno_str(ret));
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    cceph_osd* osd_ptr = *osd;
    osd_ptr->osd_id    = osd_id;
    osd_ptr->os        = os;
    osd_ptr->os_funcs  = os_funcs;
    osd_ptr->msger     = msger;
    osd_ptr->smsger    = smsger;

    return CCEPH_OK;
}

int cceph_osd_load_metadata(
    cceph_osd* osd,
    int64_t    log_id) {
    assert(log_id, osd != NULL);

    cceph_object_store* os       = osd->os;
    cceph_os_funcs*     os_funcs = osd->os_funcs;

    char*   osd_id_value = NULL;
    int32_t osd_id_value_length = 0;
    int ret = os_funcs->read_coll_map_key(os,
            CCEPH_OSD_META_COLL_ID, CCEPH_OSD_META_ATTR_OSD_ID,
            &osd_id_value_length, &osd_id_value,
            log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    assert(log_id, osd_id_value_length == sizeof(cceph_osd_id_t));
    memcpy(&osd->osd_id, osd_id_value, osd_id_value_length);

    char*   max_osdmap_epoch_value = NULL;
    int32_t max_osdmap_epoch_value_length = 0;
    ret = os_funcs->read_coll_map_key(os,
            CCEPH_OSD_META_COLL_ID, CCEPH_OSD_META_ATTR_OSDMAP_MAX_EPOCH,
            &max_osdmap_epoch_value_length, &max_osdmap_epoch_value,
            log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }
    assert(log_id, max_osdmap_epoch_value_length == sizeof(cceph_epoch_t));
    memcpy(&osd->max_osdmap_epoch, max_osdmap_epoch_value, max_osdmap_epoch_value_length);

    return CCEPH_OK;
}
int cceph_osd_load_pgs(
    cceph_osd* osd,
    int64_t    log_id) {

    assert(log_id, osd != NULL);

    return CCEPH_OK;
}

int cceph_osd_start(
        cceph_osd* osd,
        int64_t    log_id) {
    assert(log_id, osd != NULL);

    int ret = osd->os_funcs->mount(osd->os, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Mount ObjectStore Failed, errno %d(%s).",
                ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_osd_load_metadata(osd, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Load Metadata Failed, errno %d(%s).",
                ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_osd_load_pgs(osd, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Load PGs Failed, errno %d(%s).",
                ret, cceph_errno_str(ret));
        return ret;
    }

    return cceph_server_messenger_start(osd->smsger, log_id);
}

