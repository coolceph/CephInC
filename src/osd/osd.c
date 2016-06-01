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
    cceph_messenger* msger = cceph_messenger_new(
            &cceph_osd_process_message,
            *osd,
            g_cceph_option.osd_msg_workthread_count,
            log_id);
    if (msger == NULL) {
        free(*osd);
        LOG(LL_ERROR, log_id, "Initial osd error, no enough memory");
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    cceph_server_messenger *smsger = new_cceph_server_messenger(
            msger,
            g_cceph_option.osd_port_base + osd_id,
            log_id);
    if (smsger == NULL) {
        free(*osd);
        cceph_messenger_free(&msger, log_id);
        LOG(LL_ERROR, log_id, "Initial osd error, no enough memory");
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

    return CCEPH_OK;
}
int cceph_osd_load_pgs(
    cceph_osd* osd,
    int64_t    log_id) {

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

