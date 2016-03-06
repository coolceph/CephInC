extern "C" {
#include "client/client.h"
}

#include "gtest/gtest.h"

TEST(client, new_client_client) {
    cceph_osdmap osdmap;
    cceph_client *client = cceph_client_new(&osdmap);
    EXPECT_NE((cceph_client*)NULL, client);
    EXPECT_NE((cceph_messenger*)NULL, client->messenger);
    EXPECT_EQ(&osdmap, client->osdmap);
    EXPECT_EQ(client->state, CCEPH_CLIENT_STATE_UNKNOWN);
}
TEST(client, initial_client) {
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
    ret = TEST_add_req_to_wait_list(client, &header, req_count, log_id);
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
