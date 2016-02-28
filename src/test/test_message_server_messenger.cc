extern "C" {
#include "message/server_messenger.h"
#include "message/msg_write_obj.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

TEST(server_messenger, new_cceph_server_messenger) {
    cceph_messenger cceph_messenger;
    int port = 12;
    int64_t log_id = 345;

    cceph_server_messenger* server_msger = new_cceph_server_messenger(&cceph_messenger, port, log_id);
    EXPECT_NE((cceph_server_messenger*)NULL, server_msger);
    EXPECT_EQ(&cceph_messenger, server_msger->messenger);
    EXPECT_EQ(port, server_msger->port);
    EXPECT_EQ(log_id, server_msger->log_id);

    EXPECT_EQ(&cceph_messenger, cceph_server_messenger_get_messenger(server_msger));
}
