extern "C" {
#include "message/server_messenger.h"
#include "message/msg_write_obj.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

TEST(server_messenger, new_server_cceph_messenger) {
    cceph_messenger cceph_messenger;
    int port = 12;
    int64_t log_id = 345;

    server_cceph_messenger* handle = new_server_cceph_messenger(&cceph_messenger, port, log_id);
    EXPECT_NE((server_cceph_messenger*)NULL, handle);
    EXPECT_EQ(&cceph_messenger, handle->cceph_messenger);
    EXPECT_EQ(port, handle->port);
    EXPECT_EQ(log_id, handle->log_id);

    EXPECT_EQ(&cceph_messenger, get_cceph_messenger(handle));
}
