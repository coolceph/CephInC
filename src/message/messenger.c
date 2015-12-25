#include "message/messenger.h"

#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"
#include "message/msg_write_obj.h"

static conn_t* get_conn_by_fd(msg_handle_t* handle, int fd) {
    return NULL;
}
static conn_t* get_conn_by_host_and_port(msg_handle_t* handle, char* host, int port) {
    return NULL;
}

static int free_conn(msg_handle_t* handle, int fd, int64_t log_id) {
    //TODO:
    //  1) get conn from conn_list
    //  2) if conn is not found, A ERROR log and return;
    //  3) if conn is found:
    //      3.1) remove it from conn_list
    //      3.2) close fd
    //      3.3) NOTICE log
    return 0;
}

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR) 
           || (event.events & EPOLLHUP) 
           || !(event.events & EPOLLIN);
}

static msg_header* read_message(int fd, int64_t log_id) {
    LOG(LL_NOTICE, log_id, "Read Message from fd %d.", fd);

    int8_t op;
    if(read_int8(fd, &op, log_id) != 0) return NULL;

    assert(log_id, op == CCEPH_MSG_OP_WRITE);
    msg_write_obj_req* msg = malloc(sizeof(msg_write_obj_req));
    msg->header.op = op;

    if(read_string(fd, &(msg->oid_size), &(msg->oid), log_id) != 0) return NULL;
    if(read_int64(fd, &(msg->offset), log_id) != 0) return NULL;
    if(read_data(fd, &(msg->length), &(msg->data), log_id) != 0) return NULL;
    
    return (msg_header*)msg;
}

static void* start_epoll(void* arg) {
    msg_handle_t* handle = (msg_handle_t*)arg;
    int64_t log_id = handle->log_id;
    assert(log_id, handle->epoll_fd != -1);

    struct epoll_event event;
    while (1) {
        int fd_count = epoll_wait(handle->epoll_fd, &event, 1, -1);
        if (fd_count <= 0) {
            LOG(LL_ERROR, log_id, "epoll_wait return: %d", fd_count);
            continue;
        }

        int fd = event.data.fd;
        if (is_conn_err(event)) {
            LOG(LL_INFO, log_id, "Network closed for fd %d.", fd);
            free_conn(handle, fd, log_id);
            continue;
        }

        //TODO: if fd == handle->send_msg_pipe_fd[1];

        conn_t* conn = get_conn_by_fd(handle, fd);
        if (conn == NULL) {
            LOG(LL_ERROR, log_id, "epoll_wait return fd %d, but conn is not found", fd);
            continue;
        } else {
            LOG(LL_INFO, log_id, "Data received from conn %s:%d, fd %d", conn->host, conn->port, fd);
        }

        msg_header* msg = read_message(fd, log_id);
        if (msg == NULL) {
            LOG(LL_ERROR, log_id, "Read message from conn: %s:%d, fd %d error.", conn->host, conn->port, fd);
            continue;
        } else {
            LOG(LL_INFO, log_id, "Msg read from conn %s:%d, fd %d, op %d", conn->host, conn->port, fd, msg->op);
        }

        handle->msg_process(handle, conn, msg);
    }

    return NULL;
}

extern msg_handle_t* start_messager(msg_handler msg_handler, int64_t log_id) {
    //New msg_handle_t;
    msg_handle_t* handle = (msg_handle_t*)malloc(sizeof(msg_handle_t));
    handle->epoll_fd = -1;
    handle->log_id = log_id;
    handle->msg_process = msg_handler;
    handle->thread_count = 2; //TODO: we need a opinion
    handle->thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * handle->thread_count);
    init_list_head(&(handle->conn_list.node));
    init_list_head(&(handle->send_msg_list.node));

    //initial send_msg_pipe;
    int ret = pipe(handle->send_msg_pipe_fd);
    if (ret < 0) {
        LOG(LL_FATAL, log_id, "start_messager can't initial send_msg_pipe, error: %d", ret);
        free(handle->thread_ids);
        return NULL;
    }
    
    //create epoll_fd
    handle->epoll_fd = epoll_create1(0);
    if (handle->epoll_fd == -1) {
        LOG(LL_ERROR, log_id, "epoll_create error, errno: %d", errno);
        abort();
    }

    //run start_epoll by thread pool
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int i = 0; for (i = 0; i < handle->thread_count; i++) {
        //TODO: we need error handle and log
        pthread_create(handle->thread_ids + i, &thread_attr, &start_epoll, handle);
    }

    //add the send_msg_pipe_fd to epoll set
    new_conn(handle, "send_msg_pipe", 0, handle->send_msg_pipe_fd[1], log_id);
    
    return handle;
}

extern conn_t* new_conn(msg_handle_t* handle, char* host, int port, int fd, int64_t log_id) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT;
    int ret = epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl");
        abort();
    }
    //TODO: return the real conn
    return NULL;
}

