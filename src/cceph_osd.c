#include "common/errno.h"
#include "common/log.h"
#include "common/option.h"

#include "message/server_messenger.h"

#include "os/object_store.h"
#include "os/mem_store.h"

#include "osd/osd.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: cceph_osd osd_id\n");
        exit(EXIT_FAILURE);
    }
    cceph_osd_id_t osd_id = atoi(argv[1]);

    int32_t log_prefix = 200000 + osd_id;
    cceph_log_initial_id(log_prefix);
    cceph_option_init();

    int64_t log_id = cceph_log_new_id();

    cceph_mem_store *mem_store = NULL;
    int ret = cceph_mem_store_new(&mem_store, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Create MemStore failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    //TODO: This should be chose by config
    cceph_object_store os       = (cceph_object_store*)mem_store;
    cceph_os_funcs*    os_funcs = cceph_mem_store_get_funcs();

    cceph_osd* osd = NULL;
    ret = cceph_osd_initial(&osd, osd_id, os, os_funcs, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Initial OSD failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_osd_start(osd, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Start OSD failed, errno %d(%s)",
                ret, cceph_errno_str(ret));
        return ret;
    }

    return ret;
}

