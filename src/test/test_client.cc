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
    EXPECT_EQ(CCEPH_CLIENT_STATE_NORMAL, client->state);
}
