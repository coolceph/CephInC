#include "message/messager.h"

#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common/assert.h"
#include "common/log.h"

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR) 
           || (event.events & EPOLLHUP) 
           || !(event.events & EPOLLIN);
}

static msg_header* read_message(int fd, int64_t log_id) {
    LOG(LL_NOTICE, log_id, "Read Message from fd %d, call fd_process.", fd);
    return NULL;
}

static void* start_epoll(void* arg) {
    msg_handle_t* handle = (msg_handle_t*)arg;
    int64_t log_id = handle->log_id;
    assert(log_id, handle->epoll_fd == -1);

    handle->epoll_fd = epoll_create1(0);
    if (handle->epoll_fd == -1) {
        LOG(LL_ERROR, log_id, "epoll_create error, errno: %d", errno);
        abort();
    }

    struct epoll_event event;
    while (1) {
        int fd_count = epoll_wait(handle->epoll_fd, &event, 1, -1);
        if (fd_count < 0) {
            //TODO: log herer
            continue;
        }

        int fd = event.data.fd;
        if (is_conn_err(event)) {
            LOG(LL_NOTICE, log_id, "Network closed for fd %d.", fd);
            close(fd);
            continue;
        }

        msg_header* msg = read_message(fd, log_id);
        handle->msg_process(handle, msg);
    }
}

extern msg_handle_t* start_messager(int (*msg_process)(msg_handle_t*, msg_header*), 
                                    int64_t log_id) {
    //New msg_handle_t;
    msg_handle_t* handle = (msg_handle_t*)malloc(sizeof(msg_handle_t));
    handle->epoll_fd = -1;
    handle->log_id = log_id;
    handle->msg_process = msg_process;
    handle->thread_count = 2; //TODO: we need a opinion
    handle->thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * handle->thread_count);

    //run start_epoll by thread pool
    pthread_attr_t thread_attr;
    int i = 0; for (i = 0; i < handle->thread_count; i++) {
        //TODO: we need error handle and log
        pthread_create(handle->thread_ids + i, &thread_attr, &start_epoll, handle);
    }
    
    return handle;
}

extern int wait_msg(msg_handle_t* handle, int fd, int64_t log_id) {
    struct epoll_event event;
    event.data.fd = fd;
    event.data.u64 = log_id;
    event.events = EPOLLIN | EPOLLONESHOT;
    int ret = epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl");
        abort();
    }
    return ret;
}

