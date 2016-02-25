extern "C" {
#include "include/errno.h"
#include "common/log.h"
#include "message/messenger.h"
#include "message/server_messenger.h"
#include "message/msg_write_obj.h"
}

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>

#include "bhook.h"
#include "gtest/gtest.h"

char* fname_epoll_ctl = (char*)"epoll_ctl";
char* fname_close_conn = (char*)"close_conn";
char* fname_write_message = (char*)"write_message";

cceph_connection* add_conn(cceph_messenger* handle, char* host, int port, int fd) {
    cceph_connection* conn = (cceph_connection*)malloc(sizeof(cceph_connection));
    conn->port = port;
    conn->fd   = fd;
    conn->id   = fd + port;
    conn->host = (char*)malloc(sizeof(char) * strlen(host));
    conn->state = CCEPH_CONN_STATE_OPEN;
    strcpy(conn->host, host);
    pthread_mutex_init(&conn->lock, NULL);

    cceph_list_add(&conn->list_node, &handle->conn_list.list_node);
    return conn;
}
int MOCK_process_message(cceph_messenger* cceph_messenger, cceph_conn_id_t conn_id, cceph_msg_header* message, void* context) {
    EXPECT_TRUE(cceph_messenger != NULL);
    EXPECT_TRUE(conn_id > 0);
    EXPECT_TRUE(message != NULL);
    EXPECT_TRUE(context == NULL);
    return 0;
}

TEST(message_messenger, cceph_messenger_new) {
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);
    EXPECT_NE(handle, (cceph_messenger*)NULL);

    EXPECT_TRUE(handle->epoll_fd > 0);
    EXPECT_TRUE(handle->wake_thread_pipe_fd[0] > 0);
    EXPECT_TRUE(handle->wake_thread_pipe_fd[1] > 0);
    EXPECT_TRUE(handle->msg_process == &MOCK_process_message);

    EXPECT_EQ(handle->log_id, 1);
    EXPECT_EQ(cceph_atomic_get64(&handle->next_conn_id), 1);

    EXPECT_EQ(handle->thread_count, 2);
    EXPECT_NE(handle->thread_ids, (long unsigned int*)NULL);

    EXPECT_EQ(handle->conn_list.list_node.prev, &handle->conn_list.list_node);
    EXPECT_EQ(handle->conn_list.list_node.next, &handle->conn_list.list_node);
}

TEST(message_messenger, find_conn_by_id) {
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);
    cceph_connection* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    cceph_connection* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_id(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_id(handle, 9002));
    EXPECT_EQ(conn2, TEST_get_conn_by_id(handle, 9004));
}

TEST(message_messenger, find_conn_by_fd) {
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);
    cceph_connection* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    cceph_connection* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

    EXPECT_EQ(NULL, TEST_get_conn_by_fd(handle, 0));
    EXPECT_EQ(conn1, TEST_get_conn_by_fd(handle, 1));
    EXPECT_EQ(conn2, TEST_get_conn_by_fd(handle, 2));
}

TEST(message_messenger, find_conn_by_port_and_ip) {
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);
    cceph_connection* conn1 = add_conn(handle, (char*)"host1", 9001, 1);
    cceph_connection* conn2 = add_conn(handle, (char*)"host2", 9002, 2);

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
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);

    attach_and_enable_func(fname_epoll_ctl, (void*)&MOCK_new_conn_epoll_ctl);

    cceph_conn_id_t conn_id = new_conn(handle, (char*)"host1", 9001, 1, 1);
    EXPECT_TRUE(conn_id > 0);

    detach_func(fname_epoll_ctl);

	cceph_connection* conn = TEST_get_conn_by_id(handle, conn_id);
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
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);
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

int MOCK_send_msg_write_message_success(cceph_connection* conn, cceph_msg_header* msg, int64_t log_id) {
    EXPECT_TRUE(conn != NULL);
    EXPECT_TRUE(msg != NULL);
    EXPECT_EQ(2, conn->fd);
    EXPECT_EQ(1, log_id);
    return 0;
}
int MOCK_send_msg_write_message_failed(cceph_connection* conn, cceph_msg_header* msg, int64_t log_id) {
    EXPECT_TRUE(conn != NULL);
    EXPECT_TRUE(msg != NULL);
    EXPECT_EQ(2, conn->fd);
    EXPECT_EQ(1, log_id);
    return -1;
}
int MOCK_send_msg_close_conn(cceph_messenger* handle, cceph_conn_id_t id, int64_t log_id) {
    EXPECT_TRUE(handle != NULL);
    EXPECT_EQ(9004, id);
    EXPECT_EQ(1, log_id);

    cceph_connection* conn = TEST_get_conn_by_id(handle, 9004);
    EXPECT_EQ(CCEPH_CONN_STATE_CLOSED, conn->state);
    return -1;
}
TEST(message_messenger, send_msg) {
    cceph_msg_header msg;
    cceph_messenger* handle = cceph_messenger_new(&MOCK_process_message, NULL, 1);
    cceph_connection* conn = add_conn(handle, (char*)"host1", 9001, 1);
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

//This is used by the TEST_listen_thread_func
//The TEST_listen_thread_func is used both by send_and_recv & send_and_recv_with_messenger_client tests
cceph_msg_write_obj_req* get_cceph_msg_write_obj_req() {
    cceph_msg_write_obj_req *req = cceph_msg_write_obj_new();
    EXPECT_NE((cceph_msg_write_obj_req*)NULL, req);

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
    return req;
}
void expect_cceph_msg_write_obj_req(cceph_msg_write_obj_req* req) {
    EXPECT_EQ(CCEPH_MSG_OP_WRITE, req->header.op);
    EXPECT_EQ(1000, req->header.log_id);
    EXPECT_EQ(1001, req->client_id);
    EXPECT_EQ(1002, req->req_id);
    EXPECT_EQ(strlen((char*)"cceph_oid"), req->oid_size);
    EXPECT_STREQ((char*)"cceph_oid", req->oid);
    EXPECT_EQ(0, req->offset);
    EXPECT_EQ(1024, req->length);
    EXPECT_NE((char*)NULL, req->data);
}
void expect_cceph_msg_write_obj_ack(cceph_msg_write_obj_ack* ack) {
    EXPECT_EQ(CCEPH_MSG_OP_WRITE_ACK, ack->header.op);
    EXPECT_EQ(1000, ack->header.log_id);
    EXPECT_EQ(1001, ack->client_id);
    EXPECT_EQ(1002, ack->req_id);
    EXPECT_EQ(CCEPH_WRITE_OBJ_ACK_OK, ack->result);
}
int cceph_messengerr_server(cceph_messenger* cceph_messenger, cceph_conn_id_t conn_id, cceph_msg_header* header, void* context) {
    EXPECT_EQ(NULL, context);

    cceph_msg_write_obj_req* req = (cceph_msg_write_obj_req*)header;
    expect_cceph_msg_write_obj_req(req);

    cceph_msg_write_obj_ack *ack = cceph_msg_write_obj_ack_new();
    ack->header.log_id = req->header.log_id;
    ack->client_id     = req->client_id;
    ack->req_id        = req->req_id;
    ack->result        = CCEPH_WRITE_OBJ_ACK_OK;

    int64_t log_id = header->log_id;
    int ret = send_msg(cceph_messenger, conn_id, (cceph_msg_header*)ack, log_id);
    EXPECT_EQ(0, ret);

    EXPECT_EQ(0, cceph_msg_write_obj_req_free(&req, log_id));
    EXPECT_EQ(0, cceph_msg_write_obj_ack_free(&ack, log_id));

    return 0;
}
typedef struct {
    cceph_messenger* handle;
    int port;
} listen_thread_arg;
void* listen_thread_func(void* arg_ptr){
    int log_id = 123;
    listen_thread_arg* arg = (listen_thread_arg*)arg_ptr;
    cceph_messenger* handle = arg->handle;
    int port = arg->port;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(-1, listen_fd);
    
    //set server addr_param
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);

    //bind sockfd & addr
    int ret = bind(listen_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    EXPECT_NE(-1, ret);
    if (ret == -1) {
        ret = cceph_messenger_stop(handle, log_id);
        return NULL;
    }

    //listen sockfd 
    ret = listen(listen_fd, 5);
    EXPECT_NE(-1, ret);
    if (ret == -1) {
        ret = cceph_messenger_stop(handle, log_id);
        return NULL;
    }

    //have connect request use accept
    struct sockaddr their_addr;
    socklen_t len = sizeof(their_addr);
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    while(true) {
        int com_fd = accept(listen_fd, &their_addr, &len);
        EXPECT_TRUE(com_fd > 0);
        if (com_fd <= 0) {
            break;
        }

        ret = getnameinfo(&their_addr, len,
                          hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                          NI_NUMERICHOST | NI_NUMERICSERV);
        if (ret == 0) {
            LOG(LL_INFO, log_id, "Accepted connection on descriptor %d "
                                 "(host=%s, port=%s).", com_fd, hbuf, sbuf);
        } else {
            LOG(LL_ERROR, log_id, "Accepted connection on descriptor %d,"
                                  "But getnameinfo failed %d", com_fd, ret);
        }

        cceph_conn_id_t conn_id = new_conn(handle, hbuf, atoi(sbuf), com_fd, log_id);
        if (conn_id < 0) {
            LOG(LL_ERROR, log_id, "Call new_conn failed, fd %d.", com_fd);
            break;
        }
    }
    return NULL;
}
cceph_messenger* start_listen_thread(int port, int log_id) {
    cceph_messenger* handle = cceph_messenger_new(&cceph_messengerr_server, NULL, log_id);
    EXPECT_NE((cceph_messenger*)NULL, handle);

    int ret = cceph_messenger_start(handle, log_id);
    EXPECT_EQ(0, ret);

    listen_thread_arg listen_thread_arg;
    listen_thread_arg.handle = handle;
    listen_thread_arg.port = port;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_t server_thread_id;
    ret = pthread_create(&server_thread_id, &thread_attr, &listen_thread_func, &listen_thread_arg);
    EXPECT_EQ(0, ret);
    sleep(1); //for listen thread;

    return handle;
}

//TEST: send_and_recv
void TEST_cceph_msg_write_obj_req_send(int fd, pthread_mutex_t *lock, int64_t log_id) {
    pthread_mutex_lock(lock);

    cceph_msg_write_obj_req *req = get_cceph_msg_write_obj_req();

    int ret = cceph_msg_header_send(fd, &(req->header), log_id);
    EXPECT_EQ(0, ret);
    ret = cceph_msg_write_obj_req_send(fd, req, log_id);
    EXPECT_EQ(0, ret);

    EXPECT_EQ(0, cceph_msg_write_obj_req_free(&req, log_id));
    pthread_mutex_unlock(lock);
}

void TEST_cceph_msg_write_obj_ack_recv(int fd, pthread_mutex_t *lock, int64_t log_id) {
    pthread_mutex_lock(lock);
    cceph_msg_write_obj_ack *ack = cceph_msg_write_obj_ack_new();

    int ret = cceph_msg_header_recv(fd, &ack->header, log_id);
    EXPECT_EQ(0, ret);
    ret = cceph_msg_write_obj_ack_recv(fd, ack, log_id);
    EXPECT_EQ(0, ret);

    expect_cceph_msg_write_obj_ack(ack);

    ret = cceph_msg_write_obj_ack_free(&ack, log_id);
    EXPECT_EQ(0, ret);
    pthread_mutex_unlock(lock);
}

typedef struct {
    int fd;
    int count;
    pthread_mutex_t lock;
} send_and_recv_msg_arg;
void* send_and_recv_msg(void* arg_ptr) {
    send_and_recv_msg_arg *arg = (send_and_recv_msg_arg*)arg_ptr;
    int fd = arg->fd;
    int count = arg->count;;
    pthread_mutex_t *lock = &arg->lock;
    int64_t log_id = 124;

    for(int i = 0; i < count; i++) {
        TEST_cceph_msg_write_obj_req_send(fd, lock, log_id);
        TEST_cceph_msg_write_obj_ack_recv(fd, lock, log_id);
    }
    for(int i = 0; i < count; i++) {
        TEST_cceph_msg_write_obj_req_send(fd, lock, log_id);
        TEST_cceph_msg_write_obj_req_send(fd, lock, log_id);
        TEST_cceph_msg_write_obj_ack_recv(fd, lock, log_id);
        TEST_cceph_msg_write_obj_ack_recv(fd, lock, log_id);
    }
    return NULL;
}
TEST(message_messenger, send_and_recv) {
    int64_t log_id = 122;
    int port = 9000;
    cceph_messenger* handle = start_listen_thread(port, log_id);

    //Connect to Server
    int fd = socket(AF_INET,SOCK_STREAM,0);
    EXPECT_NE(0, fd);

    struct sockaddr_in their_addr;
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(port);
    inet_aton( "127.0.0.1", &their_addr.sin_addr);
    bzero(&(their_addr.sin_zero),8);

    int ret = connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
    EXPECT_NE(-1, ret);

    //Send and recv msg
    send_and_recv_msg_arg send_and_recv_msg_arg;
    send_and_recv_msg_arg.fd = fd;
    send_and_recv_msg_arg.count = 10;
    pthread_mutex_init(&(send_and_recv_msg_arg.lock), NULL);

    //Single Thread
    send_and_recv_msg(&send_and_recv_msg_arg);

    //Multi Thread
    int thread_count = 16;
    pthread_t client_thread_ids[thread_count];
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    for (int i = 0; i < thread_count; i++) {
        ret = pthread_create(client_thread_ids + i, &thread_attr, &send_and_recv_msg, &send_and_recv_msg_arg);
        EXPECT_EQ(0, ret);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(*(client_thread_ids + i), NULL);
    }

    ret = cceph_messenger_stop(handle, log_id);
    EXPECT_EQ(0, ret);

    ret = cceph_messenger_free(&handle, log_id);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(NULL, handle);
}


//TEST: send_and_recv_with_messenger_client
int cceph_messengerr_client(cceph_messenger* handle, cceph_conn_id_t conn_id, cceph_msg_header* header, void* context) {
    EXPECT_NE((void*)NULL, context);
    EXPECT_NE((cceph_messenger*)NULL, handle);
    EXPECT_TRUE(conn_id > 0);

    *((int*)context) += 1;

    cceph_msg_write_obj_ack *ack = (cceph_msg_write_obj_ack*)header;
    expect_cceph_msg_write_obj_ack(ack);

    int ret = cceph_msg_write_obj_ack_free(&ack, header->log_id);
    EXPECT_EQ(0, ret);
    return 0;
}
void* client_thread_func(void* arg) {
    EXPECT_NE((void*)NULL, arg);

    int port = *((int*)arg);
    int called_count = 0;
    int64_t log_id = pthread_self();

    cceph_messenger* handle = cceph_messenger_new(&cceph_messengerr_client, &called_count, log_id);
    EXPECT_NE((cceph_messenger*)NULL, handle);

    int ret = cceph_messenger_start(handle, log_id);
    EXPECT_EQ(0, ret);

    cceph_msg_write_obj_req *req = get_cceph_msg_write_obj_req();

    //Send msg by one conn;
    cceph_conn_id_t conn_id1 = get_conn(handle, "127.0.0.1", port, log_id);
    EXPECT_TRUE(conn_id1 > 0);
    int count = 10;
    for (int i = 0; i < count; i++) {
        ret = send_msg(handle, conn_id1, (cceph_msg_header*)req, log_id);
        EXPECT_EQ(0, ret);
    }
    while (called_count < count) ;

    //Send msg from the same conn
    called_count = 0;
    cceph_conn_id_t conn_id2 = get_conn(handle, "127.0.0.1", port, log_id);
    EXPECT_TRUE(conn_id2 > 0);
    EXPECT_EQ(conn_id1, conn_id2);
    for (int i = 0; i < count; i++) {
        ret = send_msg(handle, conn_id2, (cceph_msg_header*)req, log_id);
        EXPECT_EQ(0, ret);
    }
    while (called_count < count) ;

    ret = close_conn(handle, conn_id1, log_id);
    EXPECT_EQ(0, ret);
    ret = close_conn(handle, conn_id2, log_id);
    EXPECT_EQ(CCEPH_ERR_CONN_NOT_FOUND, ret);

    //Send msg by many conn
    count = 10;
    for (int i = 0; i < count; i++) {
        called_count = 0;

        cceph_conn_id_t conn_id = get_conn(handle, "127.0.0.1", port, log_id);
        EXPECT_TRUE(conn_id > 0);

        ret = send_msg(handle, conn_id, (cceph_msg_header*)req, log_id);
        EXPECT_EQ(0, ret);

        while (called_count < 1) ;
        ret = close_conn(handle, conn_id, log_id);
        EXPECT_EQ(0, ret);
    }

    EXPECT_EQ(0, cceph_msg_write_obj_req_free(&req, log_id));
    return NULL;
}
TEST(message_messenger, send_and_recv_with_messenger_client) {
    int64_t log_id = 122;
    int port = 9001;
    cceph_messenger* handle = start_listen_thread(port, log_id);

    //Strat Client Thread
    int thread_count = 16;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_t client_thread_ids[thread_count];
    for (int i = 0; i < thread_count; i++) {
        int ret = pthread_create(client_thread_ids + i, &thread_attr, &client_thread_func, &port);
        EXPECT_EQ(0, ret);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(*(client_thread_ids + i), NULL);
    }

    int ret = cceph_messenger_stop(handle, log_id);
    EXPECT_EQ(0, ret);

    ret = cceph_messenger_free(&handle, log_id);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(NULL, handle);
}

//TEST: use server_messenger as server
void* server_messenger_thread_func(void* arg_ptr){
    int log_id = 123;
    listen_thread_arg* arg = (listen_thread_arg*)arg_ptr;
    cceph_messenger* handle = arg->handle;
    int port = arg->port;

    server_cceph_messenger *server_cceph_messenger = new_server_cceph_messenger(handle, port, log_id);
    int ret = start_server_messenger(server_cceph_messenger, log_id);
    EXPECT_EQ(0, ret);

    return NULL;
}
TEST(server_messenger, start_server_messager) {
    int64_t log_id = 122;
    int port = 9002;
    cceph_messenger* handle = cceph_messenger_new(&cceph_messengerr_server, NULL, log_id);
    EXPECT_NE((cceph_messenger*)NULL, handle);

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    //Start Server Thread
    pthread_t server_thread_id;
    listen_thread_arg listen_thread_arg;
    listen_thread_arg.handle = handle;
    listen_thread_arg.port = port;
    int ret = pthread_create(&server_thread_id, &thread_attr, &server_messenger_thread_func, &listen_thread_arg);
    EXPECT_EQ(0, ret);
    sleep(1); //for listen thread;

    //Strat Client Thread
    int thread_count = 16;
    pthread_t client_thread_ids[thread_count];
    for (int i = 0; i < thread_count; i++) {
        int ret = pthread_create(client_thread_ids + i, &thread_attr, &client_thread_func, &port);
        EXPECT_EQ(0, ret);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(*(client_thread_ids + i), NULL);
    }

    //TODO: stop server_messenger not messenger
    ret = cceph_messenger_stop(handle, log_id);
    EXPECT_EQ(0, ret);

    ret = cceph_messenger_free(&handle, log_id);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(NULL, handle);
}
