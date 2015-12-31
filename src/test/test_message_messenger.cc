extern "C" {
#include "message/messenger.h"
}

#include "gtest/gtest.h"

TEST(message_messenger, find_conn_by_fd) {
    msg_handle_t handle;
    init_list_head(&(handle.conn_list.list_node));
    pthread_rwlock_init(&(handle.conn_list_lock), NULL);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(&handle, 0));

    conn_t conn1;
    conn1.host = (char*)"host1";
    conn1.port = 9001;
    conn1.fd   = 1;
    list_add(&conn1.list_node, &handle.conn_list.list_node);

    conn_t conn2;
    conn2.host = (char*)"host2";
    conn2.port = 9002;
    conn2.fd   = 2;
    list_add_tail(&conn2.list_node, &handle.conn_list.list_node);

    EXPECT_EQ(&conn1, TEST_get_conn_by_fd(&handle, 1));
    EXPECT_EQ(&conn2, TEST_get_conn_by_fd(&handle, 2));

}
