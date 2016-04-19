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

class os : public ::testing::Test {
public:
    cceph_object_store* GetObjectStore(int64_t log_id) {
        cceph_mem_store *store = NULL;
        int ret = cceph_mem_store_new(&store, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        return (cceph_object_store*)store;
    }
    cceph_os_funcs* GetObjectStoreFuncs() {
        return cceph_mem_store_get_funcs();
    }
};

#include "test/test_os.cc"
