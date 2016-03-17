extern "C" {
#include "message/msg_write_obj.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

#include "test/mock_message_io.h"

TEST(message_cceph_msg_write_obj_req, cceph_msg_write_obj_new) {
    cceph_msg_write_obj_req *msg = cceph_msg_write_obj_req_new();
    EXPECT_NE((cceph_msg_write_obj_req*)NULL, msg);
    EXPECT_EQ(CCEPH_MSG_OP_WRITE, msg->header.op);
    EXPECT_EQ(0, msg->header.log_id);
    EXPECT_EQ((char*)NULL, msg->oid);
    EXPECT_EQ((char*)NULL, msg->data);
}
TEST(message_cceph_msg_write_obj_req, cceph_msg_write_obj_req_free) {
    cceph_msg_write_obj_req *msg = cceph_msg_write_obj_req_new();
    cceph_msg_write_obj_req_free(&msg, 122);
    EXPECT_EQ((cceph_msg_write_obj_req*)NULL, msg);
}
TEST(message_cceph_msg_write_obj_req, cceph_msg_write_obj_req_recv) {
    attach_message_io_funcs();

    cceph_msg_write_obj_req *msg = cceph_msg_write_obj_req_new();
    int ret = cceph_msg_write_obj_req_recv(1, msg, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(32, msg->client_id);
    EXPECT_EQ(64, msg->req_id);
    EXPECT_STREQ(cceph_string, msg->oid);
    EXPECT_EQ(64, msg->offset);
    EXPECT_EQ(strlen(cceph_data), msg->length);
    EXPECT_EQ(0, strncmp(msg->data, cceph_data, strlen(cceph_data)));

    detach_message_io_funcs();
}

TEST(message_cceph_msg_write_obj_req, cceph_msg_write_obj_req_send) {
    attach_message_io_funcs();

    cceph_msg_write_obj_req msg;
    msg.client_id = 32;
    msg.req_id = 64;
    msg.oid_size = strlen(cceph_string);
    msg.oid = cceph_string;
    msg.offset = 64;
    msg.length = strlen(cceph_data);
    msg.data = cceph_data;
    int ret = cceph_msg_write_obj_req_send(1, &msg, 122);
    EXPECT_EQ(0, ret);

    detach_message_io_funcs();
}

TEST(message_cceph_msg_write_obj_ack, cceph_msg_write_obj_ack_new) {
    cceph_msg_write_obj_ack *msg = cceph_msg_write_obj_ack_new();
    EXPECT_NE((cceph_msg_write_obj_ack*)NULL, msg);
    EXPECT_EQ(CCEPH_MSG_OP_WRITE_ACK, msg->header.op);
    EXPECT_EQ(0, msg->header.log_id);
    EXPECT_EQ(0, msg->client_id);
    EXPECT_EQ(0, msg->req_id);
    EXPECT_EQ(CCEPH_WRITE_OBJ_ACK_UNKNOWN, msg->result);
}
TEST(message_cceph_msg_write_obj_ack, cceph_msg_write_obj_ack_free) {
    cceph_msg_write_obj_ack *msg = cceph_msg_write_obj_ack_new();
    cceph_msg_write_obj_ack_free(&msg, 122);
    EXPECT_EQ((cceph_msg_write_obj_ack*)NULL, msg);
}
TEST(message_cceph_msg_write_obj_ack, cceph_msg_write_obj_ack_recv) {
    attach_message_io_funcs();

    cceph_msg_write_obj_ack *msg = cceph_msg_write_obj_ack_new();
    int ret = cceph_msg_write_obj_ack_recv(1, msg, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(32, msg->client_id);
    EXPECT_EQ(64, msg->req_id);
    EXPECT_EQ(8, msg->result);

    detach_message_io_funcs();
}

TEST(message_cceph_msg_write_obj_ack, cceph_msg_write_obj_ack_send) {
    attach_message_io_funcs();

    cceph_msg_write_obj_ack msg;
    msg.client_id = 32;
    msg.req_id = 64;
    msg.result = 8;
    int ret = cceph_msg_write_obj_ack_send(1, &msg, 122);
    EXPECT_EQ(0, ret);

    detach_message_io_funcs();
}

