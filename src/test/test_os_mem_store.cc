extern "C" {
#include "common/errno.h"
#include "os/mem_store.h"
}

#include "gtest/gtest.h"


TEST(os_mem_store, cceph_mem_store_new) {
    cceph_mem_store *store = NULL;
    int ret = cceph_mem_store_new(&store, 0);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store*)NULL, store);
}

