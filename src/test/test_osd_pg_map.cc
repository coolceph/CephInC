extern "C" {
#include "common/errno.h"
#include "common/rbtree.h"

#include "osd/pg.h"
#include "osd/pg_map.h"
}

#include "gtest/gtest.h"

TEST(pg_map, insert_remove_search) {
    int64_t log_id = 122;
    cceph_rb_root root = CCEPH_RB_ROOT;
    cceph_pg *node = NULL;

    for (int i = 0; i < 1000; i++) {
        node = NULL;
        int ret = cceph_pg_new(&node, i, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_pg*)NULL, node);
        EXPECT_EQ(i, node->pg_id);
        EXPECT_EQ(CCEPH_PG_STATE_UNKNOWN, node->state);

        ret = cceph_pg_map_insert(&root, node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        node = NULL;
        int ret = cceph_pg_map_search(&root, i, &node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_pg*)NULL, node);
        EXPECT_EQ(i, node->pg_id);

        ret = cceph_pg_map_remove(&root, node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);

        node = NULL;
        ret = cceph_pg_map_search(&root, i, &node, log_id);
        EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    }
}
