#ifndef CCEPH_MESSAGE_MESSAGER_H
#define CCEPH_MESSAGE_MESSAGER_H

#include <pthread.h>

#include "common/atomic.h"
#include "common/list.h"

#include "message/msg_header.h"

#define CCEPH_CONN_STATE_UNKNOWN 0
#define CCEPH_CONN_STATE_OPEN    1
#define CCEPH_CONN_STATE_CLOSED  2

typedef int64_t conn_id_t;

typedef struct {
    conn_id_t id;

    char* host;
    int   port;
    int   fd;

    int   state;

    pthread_mutex_t lock;

    struct list_head list_node;
} connection;

typedef int8_t messenger_op_t;
#define CCEPH_MESSENGER_OP_UNKOWN  0
#define CCEPH_MESSENGER_OP_STOP    1

typedef struct msg_handle_ msg_handle_;
struct msg_handle_ {
    int log_id;
    int (*msg_process)(struct msg_handle_*, conn_id_t, msg_header*, void*);
    int *context;

    int epoll_fd;
    int thread_count;
    pthread_t *thread_ids;

    atomic64_t next_conn_id;
    connection conn_list;
    pthread_rwlock_t conn_list_lock;

    int wake_thread_pipe_fd[2]; //used to wake up thread to send msg
};

typedef msg_handle_ msg_handle;
typedef int (*msg_handler)(msg_handle*, conn_id_t, msg_header*, void*);

extern msg_handle* start_messager(msg_handler msg_handler, void* context, int64_t log_id);
extern int stop_messager(msg_handle* handle, int64_t log_id);
extern int destory_msg_handle(msg_handle** handle, int64_t log_id);

extern conn_id_t new_conn(msg_handle* handle, const char* host, int port, int fd, int64_t log_id);
extern conn_id_t get_conn(msg_handle* handle, const char* host, int port, int64_t log_id);

extern int close_conn(msg_handle* handle, conn_id_t id, int64_t log_id);

//Send msg to conn_id
//  if success return 0, else -1 and close the conn
//  this function will not free the msg
extern int send_msg(msg_handle* handle, conn_id_t conn_id, msg_header* msg, int64_t log_id);

//for test
extern msg_handle* TEST_new_msg_handle(msg_handler msg_handler, void* context, int64_t log_id);
extern connection* TEST_get_conn_by_id(msg_handle* handle, int id);
extern connection* TEST_get_conn_by_fd(msg_handle* handle, int fd);
extern connection* TEST_get_conn_by_host_and_port(msg_handle* handle, const char* host, int port);
#endif
