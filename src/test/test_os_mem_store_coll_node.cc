extern "C" {
#include "common/errno.h"
#include "os/mem_store_coll_node.h"
}

#include "gtest/gtest.h"

TEST(os_mem_store, cceph_mem_store_coll_node) {
    cceph_rb_root root = CCEPH_RB_ROOT;
    for (int i = 0; i < 1000; i++) {
        cceph_mem_store_coll_node *node = new cceph_mem_store_coll_node();
        node->cid = i;
        int ret = cceph_mem_store_coll_node_insert(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        cceph_mem_store_coll_node *node = NULL;
        int ret = cceph_mem_store_coll_node_search(&root, i, &node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_mem_store_coll_node*)NULL, node);
        EXPECT_EQ(i, node->cid);
    }
}



