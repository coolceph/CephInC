#ifndef CCEPH_MESSAGE_MESSAGER_H
#define CCEPH_MESSAGE_MESSAGER_H

#include "message/msg_header.h"

#define MAX_EPOLL_EVENT_COUNT 256

typedef struct msg_handle_t_ msg_handle_t_;

struct msg_handle_t_ {
    int (*fd_process)(struct msg_handle_t_*, int);
    int* wait_fd_list;

    int epoll_fd;
    struct epoll_event *events;
};

typedef msg_handle_t_ msg_handle_t;

extern msg_handle_t* start_messager(int (*fd_process)(msg_handle_t* handle, int fd), 
                                    int64_t log_id);

extern int send_msg(msg_handle_t* handle, msg_header* msg);

extern int wait_msg(msg_handle_t* handle, int fd);

#endif
