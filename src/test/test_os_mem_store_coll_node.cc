extern "C" {
#include "common/errno.h"
#include "os/mem_store_coll_node.h"
#include "os/mem_store_object_node.h"
}

#include "gtest/gtest.h"

TEST(os_mem_store, cceph_mem_store_coll_node_new_and_free) {
    int64_t log_id = 122;
    cceph_os_coll_id_t cid = 1;
    cceph_mem_store_coll_node* cnode = NULL;

    int ret = cceph_mem_store_coll_node_new(cid, &cnode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store_coll_node*)NULL, cnode);

    ret = cceph_mem_store_coll_node_free(&cnode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_mem_store_coll_node*)NULL, cnode);

    ret = cceph_mem_store_coll_node_new(cid, &cnode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store_coll_node*)NULL, cnode);

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_new("object", &onode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_mem_store_object_node_insert(&cnode->objects, onode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_mem_store_coll_node_free(&cnode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_mem_store_coll_node*)NULL, cnode);
}

TEST(os_mem_store, cceph_mem_store_coll_node_insert_search) {
    int64_t log_id = 122;
    cceph_rb_root root = CCEPH_RB_ROOT;
    for (int i = 0; i < 1000; i++) {
        cceph_mem_store_coll_node *cnode = NULL;
        int ret = cceph_mem_store_coll_node_new(i, &cnode, log_id);
        EXPECT_EQ(CCEPH_OK, ret);

        ret = cceph_mem_store_coll_node_insert(&root, cnode, 0);
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



