extern "C" {
#include "message/messenger.h"
#include "client/client.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

#include <pthread.h>

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
    EXPECT_EQ(0, cceph_atomic_get64(&client->req_id));
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
cceph_conn_id_t MOCK__send_req_to_osd__cceph_messenger_get_conn(
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
int MOCK__send_req_to_osd__cceph_messenger_send_msg(
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

    attach_and_enable_func_lib(fname_cceph_messenger_get_conn, (void*)&MOCK__send_req_to_osd__cceph_messenger_get_conn);
    attach_and_enable_func_lib(fname_cceph_messenger_send_msg, (void*)&MOCK__send_req_to_osd__cceph_messenger_send_msg);
    int ret = TEST_cceph_client_send_req_to_osd(&msger, &osd, &msg, log_id);
    detach_func_lib(fname_cceph_messenger_get_conn);
    detach_func_lib(fname_cceph_messenger_send_msg);

    EXPECT_EQ(0, ret);
    EXPECT_EQ(true, test_client_send_req_to_osd_param.MOCK_cceph_messenger_send_msg_called);
    EXPECT_EQ(true, test_client_send_req_to_osd_param.MOCK_cceph_messenger_get_conn_called);
}

/* extern int cceph_client_write_obj(cceph_client* client, */
/*                      char* oid, int64_t offset, int64_t length, char* data) { */

cceph_client* write_obj_client;
void* write_req_callback_thread_func(void* arg) {
    int64_t req_id = *((int64_t*)arg);
    int64_t log_id = req_id;

    cceph_msg_write_obj_ack *ack = cceph_msg_write_obj_ack_new();
    ack->header.log_id = log_id;
    ack->client_id     = 0;
    ack->req_id        = req_id;
    ack->result        = CCEPH_WRITE_OBJ_ACK_OK;

    cceph_messenger msger;
    TEST_cceph_client_process_message(&msger, req_id, &ack->header, write_obj_client);

    return 0;
}
cceph_conn_id_t MOCK__write_obj__cceph_messenger_get_conn(
        cceph_messenger* messenger, const char* host, int port, int64_t log_id) {
    EXPECT_NE((cceph_messenger*)NULL, messenger);
    EXPECT_STREQ("127.0.0.1", host);
    EXPECT_TRUE(port >= 9000 && port <= 9004);
    EXPECT_TRUE(log_id == log_id);

    return port;
}
int MOCK__write_obj__cceph_messenger_send_msg(
        cceph_messenger* messenger, cceph_conn_id_t conn_id,
        cceph_msg_header* msg, int64_t log_id) {
    EXPECT_NE((cceph_messenger*)NULL, messenger);
    EXPECT_TRUE(conn_id >= 9000 && conn_id <= 9004);
    EXPECT_EQ(CCEPH_MSG_OP_WRITE, msg->op);
    EXPECT_TRUE(log_id == log_id);

    cceph_msg_write_obj_req* req = (cceph_msg_write_obj_req*)msg;
    EXPECT_STREQ("object_1", req->oid);
    EXPECT_EQ(0, req->offset);
    EXPECT_EQ(4096, req->length);
    EXPECT_EQ(0, req->client_id);
    EXPECT_TRUE(req->req_id > 0);

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_t callback_thread_id;
    int ret = pthread_create(&callback_thread_id, &thread_attr, &write_req_callback_thread_func, &req->req_id);
    EXPECT_EQ(0, ret);

    return 0;
}

void* write_obj_thread_func(void*) {
    cceph_client *client = write_obj_client;

    char* oid = (char*)"object_1";
    int64_t offset = 0;
    int64_t length = 4096;
    char data[4096];

    int ret = cceph_client_write_obj(client, oid, offset, length, data);
    EXPECT_EQ(0, ret);
    return 0;
}

TEST(client, cceph_client_write_obj) {
    attach_and_enable_func_lib(fname_cceph_messenger_get_conn, (void*)&MOCK__write_obj__cceph_messenger_get_conn);
    attach_and_enable_func_lib(fname_cceph_messenger_send_msg, (void*)&MOCK__write_obj__cceph_messenger_send_msg);

    cceph_osdmap osdmap;
    osdmap.osd_count = 5;
    osdmap.osds = (cceph_osd_id*)malloc(sizeof(cceph_osd_id) * 5);
    osdmap.osds[0].host = (char*)"127.0.0.1";
    osdmap.osds[0].port = 9000;
    osdmap.osds[1].host = (char*)"127.0.0.1";
    osdmap.osds[1].port = 9001;
    osdmap.osds[2].host = (char*)"127.0.0.1";
    osdmap.osds[2].port = 9002;
    osdmap.osds[3].host = (char*)"127.0.0.1";
    osdmap.osds[3].port = 9003;
    osdmap.osds[4].host = (char*)"127.0.0.1";
    osdmap.osds[4].port = 9004;

    write_obj_client = cceph_client_new(&osdmap);
    cceph_client* client = write_obj_client;
    int ret = cceph_client_init(client);
    EXPECT_EQ(0, ret);

    //Single Thread
    write_obj_thread_func(client);

    //Multi Thread
    int thread_count = 16;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_t client_thread_ids[thread_count];
    for (int i = 0; i < thread_count; i++) {
        int ret = pthread_create(client_thread_ids + i, &thread_attr, &write_obj_thread_func, client);
        EXPECT_EQ(0, ret);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(*(client_thread_ids + i), NULL);
    }

    detach_func_lib(fname_cceph_messenger_get_conn);
    detach_func_lib(fname_cceph_messenger_send_msg);
}
