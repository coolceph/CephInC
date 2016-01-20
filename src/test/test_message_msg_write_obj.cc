extern "C" {
#include "message/msg_write_obj.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

#include "test/mock_message_io.h"

TEST(message_msg_write_obj_req, malloc_msg_write_obj_req) {
    msg_write_obj_req *msg = malloc_msg_write_obj_req(122);
    EXPECT_NE((msg_write_obj_req*)NULL, msg);
    EXPECT_EQ(CCEPH_MSG_OP_UNKNOWN, msg->header.op);
    EXPECT_EQ(0, msg->header.log_id);
    EXPECT_EQ((char*)NULL, msg->oid);
    EXPECT_EQ((char*)NULL, msg->data);
}
TEST(message_msg_write_obj_req, free_msg_write_obj_req) {
    msg_write_obj_req *msg = malloc_msg_write_obj_req(122);
    free_msg_write_obj_req(&msg, 122);
    EXPECT_EQ((msg_write_obj_req*)NULL, msg);
}
TEST(message_msg_write_obj_req, recv_msg_write_obj_req) {
    attach_message_io_funcs();

    msg_write_obj_req *msg = malloc_msg_write_obj_req(122);
    int ret = recv_msg_write_obj_req(1, msg, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(32, msg->client_id);
    EXPECT_EQ(32, msg->req_id);
    EXPECT_STREQ(cceph_string, msg->oid);
    EXPECT_EQ(64, msg->offset);
    EXPECT_EQ(strlen(cceph_data), msg->length);
    EXPECT_EQ(0, strncmp(msg->data, cceph_data, strlen(cceph_data)));

    detach_message_io_funcs();
}

TEST(message_msg_write_obj_req, send_msg_write_obj_req) {
    attach_message_io_funcs();

    msg_write_obj_req msg;
    msg.client_id = 32;
    msg.req_id = 32;
    msg.oid_size = strlen(cceph_string);
    msg.oid = cceph_string;
    msg.offset = 64;
    msg.length = strlen(cceph_data);
    msg.data = cceph_data;
    int ret = send_msg_write_obj_req(1, &msg, 122);
    EXPECT_EQ(0, ret);

    detach_message_io_funcs();
}

TEST(message_msg_write_obj_ack, malloc_msg_write_obj_ack) {
    msg_write_obj_ack *msg = malloc_msg_write_obj_ack(122);
    EXPECT_NE((msg_write_obj_ack*)NULL, msg);
    EXPECT_EQ(CCEPH_MSG_OP_UNKNOWN, msg->header.op);
    EXPECT_EQ(0, msg->header.log_id);
    EXPECT_EQ(0, msg->client_id);
    EXPECT_EQ(0, msg->req_id);
    EXPECT_EQ(CCEPH_WRITE_OBJ_ACK_UNKNOWN, msg->result);
}
TEST(message_msg_write_obj_ack, free_msg_write_obj_ack) {
    msg_write_obj_ack *msg = malloc_msg_write_obj_ack(122);
    free_msg_write_obj_ack(&msg, 122);
    EXPECT_EQ((msg_write_obj_ack*)NULL, msg);
}
TEST(message_msg_write_obj_ack, recv_msg_write_obj_ack) {
    attach_message_io_funcs();

    msg_write_obj_ack *msg = malloc_msg_write_obj_ack(122);
    int ret = recv_msg_write_obj_ack(1, msg, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(32, msg->client_id);
    EXPECT_EQ(32, msg->req_id);
    EXPECT_EQ(8, msg->result);

    detach_message_io_funcs();
}

TEST(message_msg_write_obj_ack, send_msg_write_obj_ack) {
    attach_message_io_funcs();

    msg_write_obj_ack msg;
    msg.client_id = 32;
    msg.req_id = 32;
    msg.result = 8;
    int ret = send_msg_write_obj_ack(1, &msg, 122);
    EXPECT_EQ(0, ret);

    detach_message_io_funcs();
}

