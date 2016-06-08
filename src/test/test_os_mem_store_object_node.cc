extern "C" {
#include "common/errno.h"
#include "os/mem_store_object_node.h"
}

#include "gtest/gtest.h"

TEST(cceph_mem_store, object_node) {
    char                         oid[256];
    int64_t                      log_id     = 122;
    cceph_rb_root                root       = CCEPH_RB_ROOT;
    cceph_mem_store_object_node* node       = NULL;

    for (int i = 0; i < 1000; i++) {
        memset(oid, 0, 256);
        sprintf(oid, "%d", i);

        node = NULL;
        int ret = cceph_mem_store_object_node_new(oid, &node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_mem_store_object_node*)NULL, node);
        EXPECT_STREQ(oid, node->oid);
        EXPECT_EQ(NULL, node->data);
        EXPECT_EQ(0, node->length);

        ret = cceph_mem_store_object_node_insert(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        memset(oid, 0, 256);
        sprintf(oid, "%d", i);

        node = NULL;
        int ret = cceph_mem_store_object_node_search(&root, oid, &node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_mem_store_object_node*)NULL, node);
        EXPECT_STREQ(oid, node->oid);
        EXPECT_EQ(NULL, node->data);
        EXPECT_EQ(0, node->length);

        ret = cceph_mem_store_object_node_remove(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);

        ret = cceph_mem_store_object_node_free(&node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_EQ((cceph_mem_store_object_node*)NULL, node);

        ret = cceph_mem_store_object_node_search(&root, oid, &node, 0);
        EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    }
}
