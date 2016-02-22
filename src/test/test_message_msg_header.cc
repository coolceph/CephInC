extern "C" {
#include "message/msg_header.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

#include "test/mock_message_io.h"

TEST(message_cceph_msg_header, cceph_msg_header_new) {
    cceph_msg_header *header = cceph_msg_header_new(122);
    EXPECT_NE((cceph_msg_header*)NULL, header);
    EXPECT_EQ(CCEPH_MSG_OP_UNKNOWN, header->op);
    EXPECT_EQ(0, header->log_id);
}
TEST(message_cceph_msg_header, cceph_msg_header_free) {
    cceph_msg_header *header = cceph_msg_header_new(122);
    cceph_msg_header_free(&header, 122);
    EXPECT_EQ((cceph_msg_header*)NULL, header);
}

TEST(message_cceph_msg_header, cceph_msg_header_recv) {
    attach_message_io_funcs();

    cceph_msg_header header;
    header.op = 0; header.log_id = 0;
    int ret = cceph_msg_header_recv(1, &header, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(8, header.op);
    EXPECT_EQ(64, header.log_id);

    detach_message_io_funcs();
}

TEST(message_cceph_msg_header, cceph_msg_header_send) {
    attach_message_io_funcs();

    cceph_msg_header header;
    header.op = 8; header.log_id = 64;
    int ret = cceph_msg_header_send(1, &header, 122);
    EXPECT_EQ(0, ret);

    detach_message_io_funcs();
}

