extern "C" {
#include "message/messenger.h"
#include <sys/epoll.h>
}

#include <pthread.h>
#include "bhook.h"
#include "gtest/gtest.h"

conn_t* add_conn(msg_handle_t* handle, char* host, int port, int fd) {
    conn_t* conn = (conn_t*)malloc(sizeof(conn_t));
    conn->port = port;
    conn->fd   = fd;
    conn->id   = fd + port;
    conn->host = (char*)malloc(sizeof(char) * strlen(host));
    strcpy(conn->host, host);
    pthread_mutex_init(&conn->lock, NULL);

    list_add(&conn->list_node, &handle->conn_list.list_node);
    return conn;
}
int MOCK_process_message(msg_handle_t* msg_handle, conn_id_t conn_id, msg_header* message) {
    EXPECT_TRUE(msg_handle != NULL);
    EXPECT_TRUE(conn_id > 0);
    EXPECT_TRUE(message != NULL);
    return 0;
}

TEST(message_messenger, new_msg_handle) {
    msg_handle_t* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    EXPECT_NE(handle, (msg_handle_t*)NULL);

    EXPECT_TRUE(handle->epoll_fd > 0);
    EXPECT_TRUE(handle->wake_thread_pipe_fd[0] > 0);
    EXPECT_TRUE(handle->wake_thread_pipe_fd[1] > 0);
    EXPECT_TRUE(handle->msg_process == &MOCK_process_message);

    EXPECT_EQ(handle->log_id, 1);
    EXPECT_EQ(atomic_get64(&handle->next_conn_id), 1);

    EXPECT_EQ(handle->thread_count, 2);
    EXPECT_NE(handle->thread_ids, (long unsigned int*)NULL);

    EXPECT_EQ(handle->conn_list.list_node.prev, &handle->conn_list.list_node);
    EXPECT_EQ(handle->conn_list.list_node.next, &handle->conn_list.list_node);
}

TEST(message_messenger, find_conn_by_id) {
    msg_handle_t* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    conn_t* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_id(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_id(handle, 9002));
    EXPECT_EQ(conn2, TEST_get_conn_by_id(handle, 9004));
}

TEST(message_messenger, find_conn_by_fd) {
    msg_handle_t* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    conn_t* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_fd(handle, 1));
    EXPECT_EQ(conn2, TEST_get_conn_by_fd(handle, 2));
}

TEST(message_messenger, find_conn_by_port_and_ip) {
    msg_handle_t* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    conn_t* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    conn_t* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_host_and_port(handle, (char*)"no_this_host", 1));
    EXPECT_EQ(conn1, TEST_get_conn_by_host_and_port(handle, (char*)"host1", 9001));
    EXPECT_EQ(conn2, TEST_get_conn_by_host_and_port(handle, (char*)"host2", 9002));
}

int MOCK_new_conn_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
	EXPECT_TRUE(epfd > 0);
	EXPECT_EQ(op, EPOLL_CTL_ADD);
	EXPECT_EQ(fd, 1);
	EXPECT_EQ(event->events, EPOLLIN | EPOLLONESHOT);
    return 0;
}
TEST(message_messenger, new_conn) {
    msg_handle_t* handle = TEST_new_msg_handle(&MOCK_process_message, 1);

    char* epoll_ctl_func_name = (char*)"epoll_ctl";
    attach_func(epoll_ctl_func_name, (void*)&MOCK_new_conn_epoll_ctl);
    fault_enable(epoll_ctl_func_name, 100, 0, NULL);

    conn_id_t conn_id = new_conn(handle, (char*)"host1", 9001, 1, 1);
    EXPECT_TRUE(conn_id > 0);

    fault_disable(epoll_ctl_func_name);
    detach_func(epoll_ctl_func_name);

	conn_t* conn = TEST_get_conn_by_id(handle, conn_id);
	EXPECT_TRUE(conn != NULL);
	EXPECT_STREQ(conn->host, "host1");
	EXPECT_EQ(conn->port, 9001);
	EXPECT_EQ(conn->fd, 1);
}

int MOCK_close_conn_close(int fd) {
    EXPECT_TRUE(fd == 1 || fd == 2);
    return 0;
}
TEST(message_messenger, close_conn) {
    msg_handle_t* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    add_conn(handle, (char*)"host1", 9001, 1);
    add_conn(handle, (char*)"host2", 9002, 2);

    char* close_func_name = (char*)"close";
    attach_func(close_func_name, (void*)&MOCK_close_conn_close);
    fault_enable(close_func_name, 100, 0, NULL);

    EXPECT_EQ(0, close_conn(handle, 9002, 1));

    fault_disable(close_func_name);
    detach_func(close_func_name);
}
