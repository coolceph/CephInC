#ifndef CCEPH_MESSAGE_MESSAGER_H
#define CCEPH_MESSAGE_MESSAGER_H

#include "message/msg_header.h"

typedef struct msg_handle_t_ msg_handle_t_;

struct msg_handle_t_ {
    int log_id;
    int (*msg_process)(struct msg_handle_t_*, msg_header*);

    int epoll_fd;
    int thread_count;
    pthread_t *thread_ids;
};

typedef msg_handle_t_ msg_handle_t;

extern msg_handle_t* start_messager(int (*msg_process)(msg_handle_t* handle, msg_header* msg), 
                                    int64_t log_id);

extern int send_msg(msg_handle_t* handle, msg_header* msg, int64_t log_id);

extern int wait_msg(msg_handle_t* handle, int fd, int64_t log_id);

#endif
