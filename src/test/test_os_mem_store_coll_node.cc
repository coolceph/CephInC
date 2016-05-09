extern "C" {
#include "common/errno.h"
#include "os/mem_store_coll_node.h"
#include "os/mem_store_object_node.h"
}

#include "gtest/gtest.h"

TEST(cceph_mem_store, coll_node) {
    int64_t                      log_id     = 122;
    cceph_rb_root                root       = CCEPH_RB_ROOT;
    cceph_mem_store_coll_node*   node       = NULL;

    for (int i = 0; i < 1000; i++) {
        node = NULL;
        int ret = cceph_mem_store_coll_node_new(i, &node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_mem_store_coll_node*)NULL, node);
        EXPECT_EQ(i, node->cid);

        ret = cceph_mem_store_coll_node_insert(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        node = NULL;
        int ret = cceph_mem_store_coll_node_search(&root, i, &node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_mem_store_coll_node*)NULL, node);
        EXPECT_EQ(i, node->cid);

        ret = cceph_mem_store_coll_node_remove(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);

        ret = cceph_mem_store_coll_node_free(&node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_EQ((cceph_mem_store_coll_node*)NULL, node);

        ret = cceph_mem_store_coll_node_search(&root, i, &node, 0);
        EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
    }
}

