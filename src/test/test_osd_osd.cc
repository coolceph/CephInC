extern "C" {
#include "common/errno.h"

#include "os/object_store.h"
#include "os/mem_store.h"

#include "osd/osd.h"
}

#include "gtest/gtest.h"

TEST(osd, initial) {
    cceph_mem_store mem_store;

    cceph_osd*          osd = NULL;
    cceph_osd_id_t      osd_id = 45;
    cceph_object_store* os = (void**)&mem_store;
    cceph_os_funcs*     funcs = cceph_mem_store_get_funcs();
    int64_t             log_id = 122;

    int ret = cceph_osd_initial(&osd, osd_id, os, funcs, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_osd*)NULL, osd);
    EXPECT_EQ(osd_id, osd->osd_id);
    EXPECT_EQ(os, osd->os);
    EXPECT_EQ(funcs, osd->os_funcs);
    EXPECT_TRUE(osd->msger != NULL);
    EXPECT_TRUE(osd->smsger != NULL);
}
