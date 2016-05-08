extern "C" {
#include "common/errno.h"
#include "os/types.h"
}

#include "gtest/gtest.h"

TEST(os_types, os_coll_id_cmp) {
    EXPECT_EQ(-1, cceph_os_coll_id_cmp(1, 2));
    EXPECT_EQ(1, cceph_os_coll_id_cmp(3, 2));
    EXPECT_EQ(0, cceph_os_coll_id_cmp(2, 2));
}

TEST(os_types, cceph_os_map_node) {
    int64_t log_id = 122;
    cceph_rb_root root = CCEPH_RB_ROOT;
    char key[256];
    cceph_os_map_node *node = NULL;

    for (int i = 0; i < 1000; i++) {
        bzero(key, 256);
        sprintf(key, "%d", i);

        node = NULL;
        int ret = cceph_os_map_node_new(key, key, strlen(key) + 1, &node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_os_map_node*)NULL, node);
        EXPECT_STREQ(key, node->key);
        EXPECT_STREQ(key, node->value);
        EXPECT_EQ(strlen(key) + 1, node->value_length);

        ret = cceph_os_map_node_insert(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        bzero(key, 256);
        sprintf(key, "%d", i);

        node = NULL;
        int ret = cceph_os_map_node_search(&root, key, &node, 0);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_os_map_node*)NULL, node);
        EXPECT_STREQ(key, node->key);
        EXPECT_STREQ(key, node->value);
        EXPECT_EQ(strlen(key) + 1, node->value_length);

        ret = cceph_os_map_node_remove(&root, node, 0);
        EXPECT_EQ(CCEPH_OK, ret);

        node = NULL;
        ret = cceph_os_map_node_search(&root, key, &node, 0);
        EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    }
}
