extern "C" {
#include "include/errno.h"
#include "message/messenger.h"
#include "message/msg_write_obj.h"
}

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <pthread.h>

#include "bhook.h"
#include "gtest/gtest.h"

char* fname_epoll_ctl = (char*)"epoll_ctl";
char* fname_close_conn = (char*)"close_conn";
char* fname_write_message = (char*)"write_message";

connection* add_conn(msg_handle* handle, char* host, int port, int fd) {
    connection* conn = (connection*)malloc(sizeof(connection));
    conn->port = port;
    conn->fd   = fd;
    conn->id   = fd + port;
    conn->host = (char*)malloc(sizeof(char) * strlen(host));
    conn->state = CCEPH_CONN_STATE_OPEN;
    strcpy(conn->host, host);
    pthread_mutex_init(&conn->lock, NULL);

    list_add(&conn->list_node, &handle->conn_list.list_node);
    return conn;
}
int MOCK_process_message(msg_handle* msg_handle, conn_id_t conn_id, msg_header* message) {
    EXPECT_TRUE(msg_handle != NULL);
    EXPECT_TRUE(conn_id > 0);
    EXPECT_TRUE(message != NULL);
    return 0;
}

TEST(message_messenger, new_msg_handle) {
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    EXPECT_NE(handle, (msg_handle*)NULL);

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
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    connection* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    connection* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_id(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_id(handle, 9002));
    EXPECT_EQ(conn2, TEST_get_conn_by_id(handle, 9004));
}

TEST(message_messenger, find_conn_by_fd) {
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    connection* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    connection* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_fd(handle, 1));
    EXPECT_EQ(conn2, TEST_get_conn_by_fd(handle, 2));
}

TEST(message_messenger, find_conn_by_port_and_ip) {
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    connection* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    connection* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

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
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);

    attach_and_enable_func(fname_epoll_ctl, (void*)&MOCK_new_conn_epoll_ctl);

    conn_id_t conn_id = new_conn(handle, (char*)"host1", 9001, 1, 1);
    EXPECT_TRUE(conn_id > 0);

    detach_func(fname_epoll_ctl);

	connection* conn = TEST_get_conn_by_id(handle, conn_id);
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
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    add_conn(handle, (char*)"host1", 9001, 1);
    add_conn(handle, (char*)"host2", 9002, 2);

    char* close_func_name = (char*)"close";
    attach_and_enable_func(close_func_name, (void*)&MOCK_close_conn_close);

    EXPECT_EQ(0, close_conn(handle, 9002, 1));
    EXPECT_EQ(NULL, TEST_get_conn_by_id(handle, 9002));
    EXPECT_EQ(CCEPH_ERR_CONN_NOT_FOUND, close_conn(handle, 9002, 1));

    EXPECT_EQ(0, close_conn(handle, 9004, 1));
    EXPECT_EQ(NULL, TEST_get_conn_by_id(handle, 9004));
    EXPECT_EQ(CCEPH_ERR_CONN_NOT_FOUND, close_conn(handle, 9004, 1));

    detach_func(close_func_name);
}

int MOCK_send_msg_write_message_success(connection* conn, msg_header* msg, int64_t log_id) {
    EXPECT_TRUE(conn != NULL);
    EXPECT_TRUE(msg != NULL);
    EXPECT_EQ(2, conn->fd);
    EXPECT_EQ(1, log_id);
    return 0;
}
int MOCK_send_msg_write_message_failed(connection* conn, msg_header* msg, int64_t log_id) {
    EXPECT_TRUE(conn != NULL);
    EXPECT_TRUE(msg != NULL);
    EXPECT_EQ(2, conn->fd);
    EXPECT_EQ(1, log_id);
    return -1;
}
int MOCK_send_msg_close_conn(msg_handle* handle, conn_id_t id, int64_t log_id) {
    EXPECT_TRUE(handle != NULL);
    EXPECT_EQ(9004, id);
    EXPECT_EQ(1, log_id);

    connection* conn = TEST_get_conn_by_id(handle, 9004);
    EXPECT_EQ(CCEPH_CONN_STATE_CLOSED, conn->state);
    return -1;
}
TEST(message_messenger, send_msg) {
    msg_header msg;
    msg_handle* handle = TEST_new_msg_handle(&MOCK_process_message, 1);
    connection* conn = add_conn(handle, (char*)"host1", 9001, 1);
    add_conn(handle, (char*)"host2", 9002, 2);

    //Case: Conn not found
    EXPECT_EQ(CCEPH_ERR_CONN_NOT_FOUND, send_msg(handle, 1, &msg, 1));

    //Case: Conn is closed
    conn->state = CCEPH_CONN_STATE_CLOSED;
    EXPECT_EQ(CCEPH_ERR_CONN_CLOSED, send_msg(handle, 9002, &msg, 1));

    //Case: Normal
    attach_and_enable_func_lib(fname_write_message, (void*)&MOCK_send_msg_write_message_success);
    EXPECT_EQ(0, send_msg(handle, 9004, &msg, 1));
    detach_func_lib(fname_write_message);

    //Case: Write failed
    attach_and_enable_func_lib(fname_write_message, (void*)&MOCK_send_msg_write_message_failed);
    attach_and_enable_func_lib(fname_close_conn, (void*)&MOCK_send_msg_close_conn);
    EXPECT_EQ(CCEPH_ERR_WRITE_CONN_ERR, send_msg(handle, 9004, &msg, 1));
    detach_func_lib(fname_write_message);
    detach_func_lib(fname_close_conn);
}

int MOCK_process_message_return_write_ack(msg_handle* msg_handle, conn_id_t conn_id, msg_header* message) {
    EXPECT_EQ(CCEPH_MSG_OP_WRITE, message->op);
    EXPECT_EQ(1000, message->log_id);

    msg_write_obj_req *req = (msg_write_obj_req*)message;
    EXPECT_EQ(1001, req->client_id);
    EXPECT_EQ(1002, req->req_id);
    EXPECT_EQ(strlen((char*)"cceph_oid"), req->oid_size);
    EXPECT_STREQ((char*)"cceph_oid", req->oid);
    EXPECT_EQ(0, req->offset);
    EXPECT_EQ(1024, req->length);
    EXPECT_NE((char*)NULL, req->data);

    msg_write_obj_ack *ack = malloc_msg_write_obj_ack();
    ack->header.log_id = message->log_id;
    ack->client_id     = req->client_id;
    ack->req_id        = req->req_id;
    ack->result        = CCEPH_WRITE_OBJ_ACK_OK;

    send_msg(msg_handle, conn_id, (msg_header*)ack, message->log_id);

    //TODO: free msg;

    return 0;
}
void* TEST_listen_thread_func(void* arg){
    int log_id = 123;
    msg_handle* handle = (msg_handle*)arg;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(-1, listen_fd);
    
    //set server addr_param
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(9000);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);

    //bind sockfd & addr
    int ret = bind(listen_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    EXPECT_NE(-1, ret);
    if (ret == -1) {
        ret = stop_messager(handle, log_id);
        return NULL;
    }

    //listen sockfd 
    ret = listen(listen_fd, 5);
    EXPECT_NE(-1, ret);
    if (ret == -1) {
        ret = stop_messager(handle, log_id);
        return NULL;
    }

    //have connect request use accept
    struct sockaddr_in their_addr;
    socklen_t len = sizeof(their_addr);
    while(true) {
        int com_fd = accept(listen_fd, (struct sockaddr*)&their_addr, &len);
        EXPECT_TRUE(com_fd > 0);
        if (com_fd <= 0) {
            break;
        }

        new_conn(handle, (char*)"TestSocket", 9001, com_fd, 122);
    }
}
void TEST_send_msg_write_obj_req(int fd, int64_t log_id) {
    msg_write_obj_req *req = malloc_msg_write_obj_req();
    EXPECT_NE((msg_write_obj_req*)NULL, req);

    req->header.log_id = 1000;
    req->client_id     = 1001;
    req->req_id        = 1002;
    req->oid_size      = strlen("cceph_oid");
    req->oid           = (char*)malloc(sizeof(char) * (req->oid_size + 1)); 
    req->offset        = 0;
    req->length        = 1024;
    req->data          = (char*)malloc(sizeof(char) * 1024);
    bzero(req->oid, req->oid_size);
    strcpy(req->oid, (char*)"cceph_oid");

    int ret = send_msg_header(fd, &(req->header), log_id);
    EXPECT_EQ(0, ret);
    ret = send_msg_write_obj_req(fd, req, log_id);
    EXPECT_EQ(0, ret);

    EXPECT_EQ(0, free_msg_write_obj_req(&req, log_id));
}
void TEST_recv_msg_write_obj_ack(int fd, int64_t log_id) {
    msg_write_obj_ack *ack = malloc_msg_write_obj_ack();
    int ret = recv_msg_write_obj_ack(fd, ack, log_id);
    EXPECT_EQ(0, ret);

    //TODO: more expect
    //TODO: free ack;
}
TEST(message_messenger, one_send_and_recv) {
    int64_t log_id = 122;
    msg_handle* handle = start_messager(&MOCK_process_message_return_write_ack, log_id);
    EXPECT_NE((msg_handle*)NULL, handle);

    //Start Listen Thread
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_t server_thread_id;
    int ret = pthread_create(&server_thread_id, &thread_attr, &TEST_listen_thread_func, handle);
    EXPECT_EQ(0, ret);
    sleep(3); //for listen thread;

    //Connect to Server
    int fd = socket(AF_INET,SOCK_STREAM,0);
    EXPECT_NE(0, fd);

    struct sockaddr_in their_addr;
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(9000);
    inet_aton( "127.0.0.1", &their_addr.sin_addr);
    bzero(&(their_addr.sin_zero),8);

    ret = connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
    EXPECT_NE(-1, ret);

    //Send and recv msg
    TEST_send_msg_write_obj_req(fd, log_id);
    TEST_recv_msg_write_obj_ack(fd, log_id);

    ret = stop_messager(handle, log_id);
    EXPECT_EQ(0, ret);
}
