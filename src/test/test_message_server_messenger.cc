extern "C" {
#include "message/server_messenger.h"
#include "message/msg_write_obj.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

TEST(server_messenger, new_cceph_server_messenger) {
    cceph_messenger cceph_messenger;
    cceph_messenger.state = CCEPH_MESSENGER_STATE_UNKNOWN;
    int port = 12;
    int64_t log_id = 345;

    cceph_server_messenger* smsger = new_cceph_server_messenger(&cceph_messenger, port, log_id);
    EXPECT_NE((cceph_server_messenger*)NULL, smsger);
    EXPECT_EQ(&cceph_messenger, smsger->messenger);
    EXPECT_EQ(port, smsger->port);
    EXPECT_EQ(log_id, smsger->log_id);

    EXPECT_EQ(&cceph_messenger, cceph_server_messenger_get_messenger(smsger));
}
