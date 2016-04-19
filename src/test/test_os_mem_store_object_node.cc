extern "C" {
#include "common/errno.h"
#include "os/mem_store_object_node.h"
}

#include "gtest/gtest.h"

TEST(os_mem_store, cceph_mem_store_object_node_new_and_free) {
    const char* oid    = "object";
    int64_t     log_id = 122;

    cceph_mem_store_object_node *node = NULL;
    int ret = cceph_mem_store_object_node_new(oid, &node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store_object_node*)NULL, node);
    EXPECT_STREQ(oid, node->oid);
    EXPECT_EQ(NULL, node->data);
    EXPECT_EQ(0, node->length);

    ret = cceph_mem_store_object_node_free(&node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_mem_store_object_node*)NULL, node);
}

TEST(os_mem_store, cceph_mem_store_object_node_insert_and_search) {
    cceph_rb_root root = CCEPH_RB_ROOT;
    char oid[256];
    for (int i = 0; i < 1000; i++) {
        bzero(oid, 256);
        sprintf(oid, "%d", i);

        cceph_mem_store_object_node *node = NULL;
        int ret = cceph_mem_store_object_node_new(oid, &node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_STREQ(oid, node->oid);
        EXPECT_EQ(NULL, node->data);
        EXPECT_EQ(0, node->length);

        ret = cceph_mem_store_object_node_insert(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
    }

    char key[256];
    for (int i = 0; i < 1000; i++) {
        bzero(key, 256);
        sprintf(key, "%d", i);
        cceph_mem_store_object_node *node = NULL;
        int ret = cceph_mem_store_object_node_search(&root, key, &node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_mem_store_object_node*)NULL, node);
        EXPECT_STREQ(key, node->oid);
    }
}
