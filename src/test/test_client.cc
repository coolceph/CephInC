extern "C" {
#include "client/client.h"
}

#include "gtest/gtest.h"

TEST(client, new_client_handle) {
    osdmap osdmap;
    client_handle *handle = new_client_handle(&osdmap);
    EXPECT_NE((client_handle*)NULL, handle);
    EXPECT_NE((msg_handle*)NULL, handle->msg_handle);
    EXPECT_EQ(&osdmap, handle->osdmap);
    EXPECT_EQ(handle->state, CCEPH_CLIENT_STATE_UNKNOWN);
}
