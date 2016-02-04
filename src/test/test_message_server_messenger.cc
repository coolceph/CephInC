extern "C" {
#include "message/server_messenger.h"
#include "message/msg_write_obj.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

TEST(server_messenger, new_server_msg_handle) {
    msg_handle msg_handle;
    int port = 12;
    int64_t log_id = 345;

    server_msg_handle* handle = new_server_msg_handle(&msg_handle, port, log_id);
    EXPECT_NE((server_msg_handle*)NULL, handle);
    EXPECT_EQ(&msg_handle, handle->msg_handle);
    EXPECT_EQ(port, handle->port);
    EXPECT_EQ(log_id, handle->log_id);
}
