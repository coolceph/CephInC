extern "C" {
#include "message/messenger.h"
}

#include "gtest/gtest.h"

void initial_msg_handle(msg_handle_t* handle) {
    init_list_head(&handle->conn_list.list_node);
    pthread_rwlock_init(&handle->conn_list_lock, NULL);
}
conn_t* add_conn(msg_handle_t* handle, char* host, int port, int fd) {
    conn_t* conn = (conn_t*)malloc(sizeof(conn_t));
    conn->host = host;
    conn->port = port;
    conn->fd   = fd;
    list_add(&conn->list_node, &handle->conn_list.list_node);

    return conn;
}

TEST(message_messenger, find_conn_by_fd) {
    msg_handle_t handle;
    initial_msg_handle(&handle);
    conn_t* conn1 = add_conn(&handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(&handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(&handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_fd(&handle, 1));
    EXPECT_EQ(conn2, TEST_get_conn_by_fd(&handle, 2));
}

TEST(message_messenger, find_conn_by_port_and_ip) {
    msg_handle_t handle;
    initial_msg_handle(&handle);
    conn_t* conn1 = add_conn(&handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(&handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_host_and_port(&handle, (char*)"no_this_host", 1));
    EXPECT_EQ(conn1, TEST_get_conn_by_host_and_port(&handle, (char*)"host1", 9001));
    EXPECT_EQ(conn2, TEST_get_conn_by_host_and_port(&handle, (char*)"host2", 9002));
}
