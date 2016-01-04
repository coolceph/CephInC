extern "C" {
#include "message/messenger.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

conn_t* add_conn(msg_handle_t* handle, char* host, int port, int fd) {
    conn_t* conn = (conn_t*)malloc(sizeof(conn_t));
    conn->host = host;
    conn->port = port;
    conn->fd   = fd;
    conn->id   = fd + port;
    list_add(&conn->list_node, &handle->conn_list.list_node);

    return conn;
}
int mock_process_message(msg_handle_t* msg_handle, conn_t* conn, msg_header* message) {
    return 0;
}

TEST(message_messenger, new_msg_handle) {
    msg_handle_t* handle = TEST_new_msg_handle(&mock_process_message, 1);
    EXPECT_NE(handle, (msg_handle_t*)NULL);

    EXPECT_TRUE(handle->epoll_fd > 0);
    EXPECT_TRUE(handle->send_msg_pipe_fd[0] > 0);
    EXPECT_TRUE(handle->send_msg_pipe_fd[1] > 0);
    EXPECT_TRUE(handle->msg_process == &mock_process_message);

    EXPECT_EQ(handle->log_id, 1);
    EXPECT_EQ(atomic_get64(&handle->next_conn_id), 1);

    EXPECT_EQ(handle->thread_count, 2);
    EXPECT_NE(handle->thread_ids, (long unsigned int*)NULL);

    EXPECT_EQ(handle->conn_list.list_node.prev, &handle->conn_list.list_node);
    EXPECT_EQ(handle->conn_list.list_node.next, &handle->conn_list.list_node);
    EXPECT_EQ(handle->send_msg_list.list_node.prev, &handle->send_msg_list.list_node);
    EXPECT_EQ(handle->send_msg_list.list_node.next, &handle->send_msg_list.list_node);
}

TEST(message_messenger, find_conn_by_id) {
    msg_handle_t* handle = TEST_new_msg_handle(&mock_process_message, 1);
    conn_t* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_id(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_id(handle, 9002));
    EXPECT_EQ(conn2, TEST_get_conn_by_id(handle, 9004));
}

TEST(message_messenger, find_conn_by_fd) {
    msg_handle_t* handle = TEST_new_msg_handle(&mock_process_message, 1);
    conn_t* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_fd(handle, 1));
    EXPECT_EQ(conn2, TEST_get_conn_by_fd(handle, 2));
}

TEST(message_messenger, find_conn_by_port_and_ip) {
    msg_handle_t* handle = TEST_new_msg_handle(&mock_process_message, 1);
    conn_t* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_host_and_port(handle, (char*)"no_this_host", 1));
    EXPECT_EQ(conn1, TEST_get_conn_by_host_and_port(handle, (char*)"host1", 9001));
    EXPECT_EQ(conn2, TEST_get_conn_by_host_and_port(handle, (char*)"host2", 9002));
}
