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

int cceph_osd_create_meta_coll(
        cceph_osd* osd,
        int64_t    log_id) {
    cceph_object_store* os       = osd->os;
    cceph_os_funcs*     os_funcs = osd->os_funcs;

    int8_t is_meta_coll_exist = 0;
    int ret = os_funcs->exist_coll(os, CCEPH_OS_META_COLL_ID, &is_meta_coll_exist, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Call ExistColl on MetaColl Failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }
    if (is_meta_coll_exist) {
        LOG(LL_ERROR, log_id, "MetaColl already existed, cannot create osd");
        return CCEPH_ERR_COLL_ALREADY_EXIST;
    }

    cceph_os_tran* tran = NULL;
    ret = cceph_os_tran_new(&tran, log_id);
    if (ret != CCEPH_OK) return ret;
    ret = cceph_os_coll_create(tran, CCEPH_OS_META_COLL_ID, log_id);
    if (ret != CCEPH_OK) return ret;
    ret = os_funcs->submit_tran(os, tran, log_id);
    if (ret != CCEPH_OK) return ret;

    return CCEPH_OK;
}
int cceph_osd_create(
        cceph_osd* osd,
        int64_t    log_id) {

    assert(log_id, osd != NULL);

    int ret = cceph_osd_create_meta_coll(osd, log_id);
    if (ret != CCEPH_OK) {
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

int cceph_osd_load_metadata(
    cceph_osd* osd,
    int64_t    log_id) {

    assert(log_id, osd != NULL);

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

