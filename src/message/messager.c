#include "message/messager.h"

#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common/log.h"

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR) 
           || (event.events & EPOLLHUP) 
           || !(event.events & EPOLLIN);
}

static void start_epoll(void* arg, int64_t log_id) {
    msg_handle_t* handle = (msg_handle_t*)arg;
    assert(log_id, handle->epoll_fd == 0);

    handle->epoll_fd = epoll_create1(0);
    if (handle->epoll_fd == -1) {
        LOG(LL_ERROR, log_id, "epoll_create error, errno: %d", errno);
        abort();
    }

    while (1) {
        int fd_count = epoll_wait(handle->epoll_fd, handle->events, MAX_EPOLL_EVENT_COUNT, -1);
        int i = 0;
        for (i = 0; i < fd_count; i++) {
            int fd = handle->events[i].data.fd;
            if(is_conn_err(handle->events[i])) {
                LOG(LL_NOTICE, log_id, "Network closed for fd %d.", fd);
                close(fd);
                continue;
            } 
            
            LOG(LL_NOTICE, log_id, "New data from fd %d, call fd_process.", fd);
            handle->fd_process(handle, fd);
        }
    }
}

extern msg_handle_t* start_messager(int (*fd_process)(msg_handle_t* handle, int fd), 
                                    int64_t log_id) {
    //New msg_handle_t;
    msg_handle_t* handle = (msg_handle_t*)malloc(sizeof(msg_handle_t));
    handle->epoll_fd = -1;
    handle->fd_process = fd_process;
    handle->events = malloc(MAX_EPOLL_EVENT_COUNT * sizeof(struct epoll_event));

    //run start_epoll in anther thread
    
    return handle;
}

