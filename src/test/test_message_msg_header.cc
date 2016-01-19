extern "C" {
#include "message/msg_header.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

#include "test/mock_message_io.h"

TEST(message_msg_header, malloc_msg_header) {
    msg_header *header = malloc_msg_header(122);
    EXPECT_NE((msg_header*)NULL, header);
    EXPECT_EQ(CCEPH_MSG_OP_UNKNOWN, header->op);
    EXPECT_EQ(0, header->log_id);
}
TEST(message_msg_header, free_msg_header) {
    msg_header *header = malloc_msg_header(122);
    free_msg_header(&header, 122);
    EXPECT_EQ((msg_header*)NULL, header);
}

TEST(message_msg_header, recv_msg_header) {
    attach_message_io_funcs();

    msg_header header;
    header.op = 0; header.log_id = 0;
    int ret = recv_msg_header(1, &header, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(8, header.op);
    EXPECT_EQ(64, header.log_id);

    detach_message_io_funcs();
}

TEST(message_msg_header, send_msg_header) {
    attach_message_io_funcs();

    msg_header header;
    header.op = 8; header.log_id = 64;
    int ret = send_msg_header(1, &header, 122);
    EXPECT_EQ(0, ret);

    detach_message_io_funcs();
}

