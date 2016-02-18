extern "C" {
#include "client/client.h"
}

#include "gtest/gtest.h"

TEST(client, new_client_handle) {
    osdmap osdmap;
    client_handle *handle = cceph_new_client_handle(&osdmap);
    EXPECT_NE((client_handle*)NULL, handle);
    EXPECT_NE((msg_handle*)NULL, handle->msg_handle);
    EXPECT_EQ(&osdmap, handle->osdmap);
    EXPECT_EQ(handle->state, CCEPH_CLIENT_STATE_UNKNOWN);
}
TEST(client, initial_client) {
    osdmap osdmap;
    client_handle *handle = cceph_new_client_handle(&osdmap);

    int ret = cceph_initial_client(handle);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(CCEPH_CLIENT_STATE_NORMAL, handle->state);
}
