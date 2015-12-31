extern "C" {
#include "message/messenger.h"
}

#include "gtest/gtest.h"

TEST(message_messenger, find_conn_by_fd) {
    msg_handle_t handle;
    init_list_head(&(handle.conn_list.conn_list_node));
    pthread_rwlock_init(&(handle.conn_list_lock), NULL);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(&handle, 0));
}
