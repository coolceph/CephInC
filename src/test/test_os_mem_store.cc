extern "C" {
#include "common/errno.h"
#include "os/mem_store.h"
}

#include "gtest/gtest.h"

TEST(os_mem_store, cceph_mem_store_coll_node) {
    cceph_rb_root root = CCEPH_RB_ROOT;
    for (int i = 0; i < 1000; i++) {
        cceph_mem_store_coll_node *node = new cceph_mem_store_coll_node();
        node->cid = i;
        int ret = TEST_cceph_mem_store_coll_node_insert(&root, node);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        cceph_mem_store_coll_node *node = TEST_cceph_mem_store_coll_node_search(&root, i);
        EXPECT_NE((cceph_mem_store_coll_node*)NULL, node);
        EXPECT_EQ(i, node->cid);
    }
}

TEST(os_mem_store, cceph_mem_store_new) {
    cceph_mem_store *store = cceph_mem_store_new();
    EXPECT_NE((cceph_mem_store*)NULL, store);
}

