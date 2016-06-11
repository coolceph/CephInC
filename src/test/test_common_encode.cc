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
