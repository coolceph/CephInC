#ifndef CCEPH_MESSAGE_MESSAGER_H
#define CCEPH_MESSAGE_MESSAGER_H

#include "common/list.h"
#include "message/msg_header.h"

typedef struct {
    char* host;
    int   port;
    int   fd;

    pthread_mutex_t write_lock;

    struct list_head node;
} conn_t;

typedef struct msg_handle_t_ msg_handle_t_;
struct msg_handle_t_ {
    int log_id;
    int (*msg_process)(struct msg_handle_t_*, conn_t* conn, msg_header*);

    int epoll_fd;
    int thread_count;
    pthread_t *thread_ids;

    conn_t conn_list;
    pthread_rwlock_t conn_list_lock;

    msg_header send_msg_list; //send_msg will put msg here
    pthread_mutex_t send_msg_list_lock;

    int send_msg_pipe_fd[2]; //used to wake up thread to send msg
};

typedef msg_handle_t_ msg_handle_t;
typedef int (*msg_handler)(msg_handle_t*, conn_t*, msg_header*);

extern msg_handle_t* start_messager(msg_handler msg_handler, int64_t log_id);

extern conn_t* new_conn(msg_handle_t* handle, char* host, int port, int fd, int64_t log_id);
extern conn_t* get_conn(msg_handle_t* handle, char* host, int port, int64_t log_id);

extern int send_msg(msg_handle_t* handle, conn_t* conn, msg_header* msg, int64_t log_id);
extern int wait_msg(msg_handle_t* handle, conn_t* conn, int64_t log_id);

#endif
