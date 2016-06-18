extern "C" {
#include "common/encode.h"
#include "common/errno.h"
}

#include "gtest/gtest.h"

TEST(encode, type) {
    cceph_buffer* buffer = NULL;
    int64_t log_id = 122;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    int32_t value = 234;
    ret = cceph_encode_int32_t(buffer, value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(sizeof(value), buffer->offset);

    int32_t result = 0;
    cceph_buffer_node* node = buffer->head;
    memcpy(&result, node->data, sizeof(result));
    EXPECT_EQ(value, result);
}
TEST(decode, type) {
    cceph_buffer*        buffer = NULL;
    cceph_buffer_reader* reader = NULL;
    int32_t value  = 234;
    int32_t result = 0;
    int64_t log_id = 122;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_encode_int32_t(buffer, value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_decode_int32_t(reader, &result, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(value, result);
}
