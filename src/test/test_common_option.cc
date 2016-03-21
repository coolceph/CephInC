extern "C" {
#include "common/option.h"
}

#include "gtest/gtest.h"

TEST(common_option, cceph_option_init) {
    int ret = cceph_option_init();
    EXPECT_EQ(0, ret);

    EXPECT_EQ(2, g_cceph_option.client_msg_workthread_count);
    EXPECT_EQ(1, g_cceph_option.client_debug_check_duplicate_req_when_ack);

    EXPECT_EQ(2, g_cceph_option.osd_msg_workthread_count);
    EXPECT_EQ(1, g_cceph_option.osd_reply_write_commit_to_client);
}
