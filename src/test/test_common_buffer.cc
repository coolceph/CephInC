extern "C" {
#include "common/buffer.h"

#include "common/errno.h"
}

#include "gtest/gtest.h"

TEST(buffer, new_and_free) {
    cceph_buffer* buffer = NULL;
    int64_t       log_id = 122;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_buffer*)NULL, buffer);
    EXPECT_NE((cceph_buffer_node*)NULL, buffer->head);
    EXPECT_EQ(buffer->head, buffer->current);
    EXPECT_EQ(CCEPH_BUFFER_NODE_LENGTH, buffer->length);
    EXPECT_EQ(0, buffer->offset);

    cceph_buffer_node* node = buffer->head;
    EXPECT_EQ(node->data, node->ptr);
    EXPECT_EQ(CCEPH_BUFFER_NODE_LENGTH, node->length);
    EXPECT_EQ((cceph_buffer_node*)NULL, node->next);

    ret = cceph_buffer_free(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_buffer*)NULL, buffer);
}

