extern "C" {
#include "message/messenger.h"
#include "client/client.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

char* fname_cceph_messenger_get_conn = (char*)"cceph_messenger_get_conn";
char* fname_cceph_messenger_send_msg = (char*)"cceph_messenger_send_msg";

TEST(client, cceph_client_new) {
    cceph_osdmap osdmap;
    cceph_client *client = cceph_client_new(&osdmap);
    EXPECT_NE((cceph_client*)NULL, client);
    EXPECT_NE((cceph_messenger*)NULL, client->messenger);
    EXPECT_EQ(&osdmap, client->osdmap);
    EXPECT_EQ(client->state, CCEPH_CLIENT_STATE_UNKNOWN);
}
TEST(client, cceph_client_init) {
    cceph_osdmap osdmap;
    cceph_client *client = cceph_client_new(&osdmap);

    int ret = cceph_client_init(client);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, client->client_id);
    EXPECT_EQ(0, cceph_atomic_get(&client->req_id));
    EXPECT_EQ(CCEPH_CLIENT_STATE_NORMAL, client->state);
}
TEST(client, add_req_to_wait_list) {
    cceph_osdmap osdmap;
    cceph_client *client = cceph_client_new(&osdmap);
    int ret = cceph_client_init(client);
    EXPECT_EQ(0, ret);

    cceph_msg_header header;
    int req_count = 3;
    int64_t log_id = 1;
    ret = TEST_cceph_client_add_req_to_wait_list(client, &header, req_count, log_id);
    EXPECT_EQ(0, ret);

    struct cceph_list_head *pos;
    cceph_client_wait_req *wait_req = NULL;
    cceph_list_for_each(pos, &(client->wait_req_list.list_node)) {
        wait_req = cceph_list_entry(pos, cceph_client_wait_req, list_node);
        EXPECT_EQ(&header, wait_req->req);
        EXPECT_EQ(3, wait_req->req_count);
        EXPECT_EQ(0, wait_req->ack_count);
        EXPECT_EQ(0, wait_req->commit_count);
    }

}

struct test_client_send_req_to_osd_param_type {
    cceph_messenger* msger;
    cceph_msg_header *msg;
    cceph_conn_id_t conn_id;
    char* host;
    int port;
    int64_t log_id;

    bool MOCK_cceph_messenger_get_conn_called;
    bool MOCK_cceph_messenger_send_msg_called;
} test_client_send_req_to_osd_param;
cceph_conn_id_t MOCK_cceph_messenger_get_conn(
        cceph_messenger* messenger, const char* host, int port, int64_t log_id) {
    EXPECT_EQ(test_client_send_req_to_osd_param.msger, messenger);
    EXPECT_STREQ(test_client_send_req_to_osd_param.host, host);
    EXPECT_EQ(test_client_send_req_to_osd_param.port, port);
    EXPECT_EQ(test_client_send_req_to_osd_param.log_id, log_id);

    cceph_conn_id_t conn_id = 456;
    test_client_send_req_to_osd_param.conn_id = conn_id;
    test_client_send_req_to_osd_param.MOCK_cceph_messenger_get_conn_called = true;
    return conn_id;
}
int MOCK_cceph_messenger_send_msg(
        cceph_messenger* messenger, cceph_conn_id_t conn_id,
        cceph_msg_header* msg, int64_t log_id) {
    EXPECT_EQ(test_client_send_req_to_osd_param.msger, messenger);
    EXPECT_EQ(test_client_send_req_to_osd_param.conn_id, conn_id);
    EXPECT_EQ(test_client_send_req_to_osd_param.msg, msg);
    EXPECT_EQ(test_client_send_req_to_osd_param.log_id, log_id);

    test_client_send_req_to_osd_param.MOCK_cceph_messenger_send_msg_called = true;
    return 0;
}

TEST(client, send_req_to_osd) {
    cceph_messenger msger;
    cceph_msg_header msg;
    cceph_osd_id osd;
    osd.host = (char*)"127.0.0.1";
    osd.port = 9000;
    int64_t log_id = 34;

    test_client_send_req_to_osd_param.msger = &msger;
    test_client_send_req_to_osd_param.msg = &msg;
    test_client_send_req_to_osd_param.host = osd.host;
    test_client_send_req_to_osd_param.port = osd.port;
    test_client_send_req_to_osd_param.log_id = log_id;
    test_client_send_req_to_osd_param.MOCK_cceph_messenger_get_conn_called = false;
    test_client_send_req_to_osd_param.MOCK_cceph_messenger_send_msg_called = false;

    attach_and_enable_func_lib(fname_cceph_messenger_get_conn, (void*)&MOCK_cceph_messenger_get_conn);
    attach_and_enable_func_lib(fname_cceph_messenger_send_msg, (void*)&MOCK_cceph_messenger_send_msg);
    int ret = TEST_cceph_client_send_req_to_osd(&msger, &osd, &msg, log_id);
    detach_func_lib(fname_cceph_messenger_get_conn);
    detach_func_lib(fname_cceph_messenger_send_msg);

    EXPECT_EQ(0, ret);
    EXPECT_EQ(true, test_client_send_req_to_osd_param.MOCK_cceph_messenger_send_msg_called);
    EXPECT_EQ(true, test_client_send_req_to_osd_param.MOCK_cceph_messenger_get_conn_called);
}
