extern "C" {
#include "common/osdmap.h"

#include "common/buffer.h"
#include "common/errno.h"
#include "common/encode.h"
}

#include "gtest/gtest.h"

TEST(cceph_osd_entity, encode_and_decode) {
    cceph_buffer*        buffer = NULL;
    cceph_buffer_reader* reader = NULL;
    cceph_osd_entity value;
    cceph_osd_entity result;
    int64_t log_id = 122;

    value.id = 1;
    result.id = 0;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_encode_osd_entity(buffer, &value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_decode_osd_entity(reader, &result, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(value.id, result.id);
}
TEST(cceph_osdmap, encode_and_decode) {
    cceph_buffer*        buffer = NULL;
    cceph_buffer_reader* reader = NULL;
    cceph_osdmap value;
    cceph_osdmap result;
    cceph_osd_entity osds[3];
    int64_t log_id = 122;

    value.epoch     = 1;
    value.pg_count  = 256;
    value.osd_count = 3;
    value.osds      = &osds[0];
    osds[0].id      = 0;
    osds[1].id      = 1;
    osds[2].id      = 2;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_encode_osdmap(buffer, &value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_decode_osdmap(reader, &result, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    EXPECT_EQ(value.epoch, result.epoch);
    EXPECT_EQ(value.pg_count, result.pg_count);
    EXPECT_EQ(value.osd_count, result.osd_count);
    for (int i = 0; i < value.osd_count; i++) {
        EXPECT_EQ(value.osds[i].id, result.osds[i].id);
    }
}

TEST(osdmap_map, insert_remove_search) {
    int64_t log_id = 122;
    cceph_rb_root root = CCEPH_RB_ROOT;
    cceph_osdmap *node = NULL;

    for (int i = 0; i < 1000; i++) {
        node = (cceph_osdmap*)malloc(sizeof(cceph_osdmap));
        node->epoch = i;
        int ret = cceph_osdmap_map_insert(&root, node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
    }
    for (int i = 0; i < 1000; i++) {
        node = NULL;
        int ret = cceph_osdmap_map_search(&root, i, &node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        EXPECT_NE((cceph_osdmap*)NULL, node);
        EXPECT_EQ(i, node->epoch);

        ret = cceph_osdmap_map_remove(&root, node, log_id);
        EXPECT_EQ(CCEPH_OK, ret);

        node = NULL;
        ret = cceph_osdmap_map_search(&root, i, &node, log_id);
        EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    }
}
