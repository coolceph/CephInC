extern "C" {
#include "common/errno.h"

#include "os/object_store.h"
#include "os/mem_store.h"

#include "osd/osd.h"
#include "osd/osd_create.h"
}

#include "gtest/gtest.h"

TEST(osd, create) {
    int64_t             log_id = 122;
    cceph_osd*          osd = NULL;
    cceph_osd_id_t      osd_id = 45;
    cceph_mem_store*    store = NULL;
    cceph_os_funcs*     funcs = cceph_mem_store_get_funcs();

    int ret = cceph_mem_store_new(&store, 0);
    EXPECT_EQ(CCEPH_OK, ret);

    cceph_object_store* os = (cceph_object_store*)store;
    ret = cceph_osd_initial(&osd, osd_id, os, funcs, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_osd_create(osd, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    cceph_osd_id_t* result_osd_id = NULL;
    int result_osd_id_length = 0;
    ret = funcs->read_coll_map_key(os,
            CCEPH_OS_META_COLL_ID, CCEPH_OS_META_ATTR_OSD_ID,
            &result_osd_id_length, (char**)&result_osd_id, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(sizeof(cceph_osd_id_t), result_osd_id_length);
    EXPECT_EQ(45, *result_osd_id);

    cceph_epoch_t* result_max_osdmap_epoch = NULL;
    int result_max_osdmap_epoch_length = 0;
    ret = funcs->read_coll_map_key(os,
            CCEPH_OS_META_COLL_ID, CCEPH_OS_META_ATTR_OSDMAP_MAX_EPOCH,
            &result_max_osdmap_epoch_length, (char**)&result_max_osdmap_epoch, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(sizeof(cceph_epoch_t), result_max_osdmap_epoch_length);
    EXPECT_EQ(0, *result_max_osdmap_epoch);
}
