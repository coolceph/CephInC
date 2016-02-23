extern "C" {
#include "client/client.h"
}

#include "gtest/gtest.h"

TEST(client, new_client_handle) {
    cceph_osdmap osdmap;
    cceph_client_handle *handle = cceph_client_handle_new(&osdmap);
    EXPECT_NE((cceph_client_handle*)NULL, handle);
    EXPECT_NE((msg_handle*)NULL, handle->msg_handle);
    EXPECT_EQ(&osdmap, handle->osdmap);
    EXPECT_EQ(handle->state, CCEPH_CLIENT_STATE_UNKNOWN);
}
TEST(client, initial_client) {
    cceph_osdmap osdmap;
    cceph_client_handle *handle = cceph_client_handle_new(&osdmap);

    int ret = cceph_client_initial(handle);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(CCEPH_CLIENT_STATE_NORMAL, handle->state);
}
