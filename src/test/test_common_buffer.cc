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

TEST(buffer, append) {
    cceph_buffer* buffer = NULL;
    int64_t       log_id = 122;
    cceph_buffer_node* node;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    char data[8192];
    memset(data, 'a', 8192);

    //Append 1 byte
    ret = cceph_buffer_append(buffer, data, 1, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(CCEPH_BUFFER_NODE_LENGTH, buffer->length);
    EXPECT_EQ(1, buffer->offset);
    EXPECT_EQ(buffer->current, buffer->head);

    node = buffer->head;
    EXPECT_EQ(node->data + 1, node->ptr);
    EXPECT_EQ((cceph_buffer_node*)NULL, node->next);
    EXPECT_EQ('a', *(char*)node->data);

    //Append 4096 bytes
    ret = cceph_buffer_append(buffer, data, 4096, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(CCEPH_BUFFER_NODE_LENGTH * 2, buffer->length);
    EXPECT_EQ(4097, buffer->offset);
    EXPECT_EQ(buffer->current, buffer->head->next);

    node = buffer->current;
    EXPECT_EQ(node->data + 1, node->ptr);
    EXPECT_EQ((cceph_buffer_node*)NULL, node->next);
    EXPECT_EQ('a', *(char*)node->data);

    //Append 8192 bytes
    ret = cceph_buffer_append(buffer, data, 8192, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(CCEPH_BUFFER_NODE_LENGTH * 4, buffer->length);
    EXPECT_EQ(1 + 4096 + 8192, buffer->offset);
    EXPECT_EQ(buffer->current, buffer->head->next->next->next);

    node = buffer->current;
    EXPECT_EQ(node->data + 1, node->ptr);
    EXPECT_EQ((cceph_buffer_node*)NULL, node->next);
    EXPECT_EQ('a', *(char*)node->data);

    //Free
    ret = cceph_buffer_free(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_buffer*)NULL, buffer);
}
TEST(buffer_reader, new_and_free) {
    cceph_buffer*        buffer = NULL;
    cceph_buffer_reader* reader = NULL;
    int64_t              log_id = 122;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_buffer_reader*)NULL, reader);
    EXPECT_EQ(buffer, reader->buffer);
    EXPECT_EQ(buffer->head, reader->node);
    EXPECT_EQ(buffer->head->data, reader->ptr);

    ret = cceph_buffer_reader_free(&reader, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_buffer_reader*)NULL, reader);

    ret = cceph_buffer_free(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_buffer*)NULL, buffer);
}

TEST(buffer_reader, read) {
    cceph_buffer*        buffer = NULL;
    cceph_buffer_reader* reader = NULL;
    int64_t              log_id = 122;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    char input[9000], output[9001];
    memset(input, 'a', 9000);
    ret = cceph_buffer_append(buffer, input, 1, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_buffer_reader_read(reader, output, 2, log_id);
    EXPECT_EQ(CCEPH_ERR_BUFFER_END, ret);

    reader = NULL;
    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_buffer_reader_read(reader, output, 1, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ('a', output[0]);
    ret = cceph_buffer_reader_read(reader, output, 1, log_id);
    EXPECT_EQ(CCEPH_ERR_BUFFER_END, ret);

    ret = cceph_buffer_append(buffer, input, 9000, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    reader = NULL;
    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_buffer_reader_read(reader, output, 9001, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    for (int i = 0; i < 9001; i++) {
        EXPECT_EQ('a', output[i]);
    }
    ret = cceph_buffer_reader_read(reader, output, 1, log_id);
    EXPECT_EQ(CCEPH_ERR_BUFFER_END, ret);
}
