extern "C" {
#include "common/errno.h"
#include "common/rbtree.h"
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
    const char* key = "oid";
    cceph_os_map_node *node = NULL;

    int ret = cceph_os_map_node_new(key, key, strlen(key) + 1, &node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_os_map_node*)NULL, node);
    EXPECT_STREQ(key, node->key);
    EXPECT_STREQ(key, node->value);
    EXPECT_EQ(strlen(key) + 1, node->value_length);

    ret = cceph_os_map_node_free(&node, log_id);
    EXPECT_EQ((cceph_os_map_node*)NULL, node);
}

TEST(os_types, cceph_os_map_update) {
    cceph_rb_root      result_tree = CCEPH_RB_ROOT;
    cceph_rb_root      input_tree  = CCEPH_RB_ROOT;

    cceph_os_map_node* node = NULL;
    int ret = cceph_os_map_node_new("key1", "value1", strlen("value1"), &node, 122);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_map_node_map_insert(&input_tree, node, 122);
    EXPECT_EQ(CCEPH_OK, ret);

    //Add Node
    ret = cceph_os_map_update(&result_tree, &input_tree, 122);
    EXPECT_EQ(CCEPH_OK, ret);
    cceph_os_map_node* node2 = NULL;
    ret = cceph_os_map_node_map_search(&result_tree, "key1", &node2, 122);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_STREQ("key1", node2->key);
    EXPECT_STREQ("value1", node2->value);
    EXPECT_EQ(strlen("value1"), node2->value_length);

    //Update Node
    node->value = (char*)"value1_changed";
    node->value_length = strlen("value1_changed");
    ret = cceph_os_map_update(&result_tree, &input_tree, 122);
    EXPECT_EQ(CCEPH_OK, ret);
    cceph_os_map_node* node3 = NULL;
    ret = cceph_os_map_node_map_search(&result_tree, "key1", &node3, 122);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_STREQ("key1", node3->key);
    EXPECT_STREQ("value1_changed", node3->value);
    EXPECT_EQ(strlen("value1_changed"), node3->value_length);

    //Remove Node
    node->value = NULL;
    node->value_length = 0;
    ret = cceph_os_map_update(&result_tree, &input_tree, 122);
    EXPECT_EQ(CCEPH_OK, ret);
    cceph_os_map_node* node4 = NULL;
    ret = cceph_os_map_node_map_search(&result_tree, "key1", &node4, 122);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
}
