#ifndef CCEPH_MESSAGE_MESSAGER_H
#define CCEPH_MESSAGE_MESSAGER_H

#include <pthread.h>

#include "common/atomic.h"
#include "common/list.h"

#include "message/msg_header.h"

#define CCEPH_CONN_STATE_UNKNOWN 0
#define CCEPH_CONN_STATE_OPEN    1
#define CCEPH_CONN_STATE_CLOSED  2

typedef int64_t cceph_conn_id_t;

typedef struct {
    cceph_conn_id_t id;

    char* host;
    int   port;
    int   fd;

    int   state;

    pthread_mutex_t lock;

    struct cceph_list_head list_node;
} cceph_connection;

typedef int8_t cceph_messenger_op_t;
#define CCEPH_MESSENGER_OP_UNKOWN  0
#define CCEPH_MESSENGER_OP_STOP    1

typedef int8_t cceph_messenger_state_t;
#define CCEPH_MESSENGER_STATE_UNKNOWN  0
#define CCEPH_MESSENGER_STATE_NORMAL   1
#define CCEPH_MESSENGER_STATE_DESTORY  2

typedef struct cceph_messenger_ cceph_messenger_;
struct cceph_messenger_ {
    cceph_messenger_state_t state;
    int log_id;

    int (*msg_process)(struct cceph_messenger_*,
            cceph_conn_id_t, cceph_msg_header*, void*);
    int *context;

    int epoll_fd;
    int thread_count;
    pthread_t *thread_ids;

    cceph_atomic64_t next_conn_id;
    cceph_connection conn_list;
    pthread_rwlock_t conn_list_lock;

    int wake_thread_pipe_fd[2]; //used to wake up thread to send msg
};

typedef cceph_messenger_ cceph_messenger;
typedef int (*cceph_msg_messengerr)(cceph_messenger*,
        cceph_conn_id_t, cceph_msg_header*, void*);

extern cceph_messenger* cceph_messenger_new(
        cceph_msg_messengerr msg_messengerr, void* context, int64_t log_id);
extern int cceph_messenger_free(
        cceph_messenger** messenger, int64_t log_id);

extern int cceph_messenger_start(
        cceph_messenger* cceph_messenger, int64_t log_id);
extern int cceph_messenger_stop(
        cceph_messenger* messenger, int64_t log_id);

extern cceph_conn_id_t cceph_messenger_add_conn(
        cceph_messenger* messenger, const char* host, int port, int fd, int64_t log_id);
extern cceph_conn_id_t cceph_messenger_get_conn(
        cceph_messenger* messenger, const char* host, int port, int64_t log_id);

extern int cceph_messenger_close_conn(
        cceph_messenger* messenger, cceph_conn_id_t id, int64_t log_id);

//Send msg to cceph_conn_id
//  if success return 0, else -1 and close the conn
//  this function will not free the msg
extern int cceph_messenger_send_msg(
        cceph_messenger* messenger, cceph_conn_id_t conn_id,
        cceph_msg_header* msg, int64_t log_id);

//for test
extern cceph_connection* TEST_cceph_messenger_get_conn_by_id(
        cceph_messenger* messenger, int id);
extern cceph_connection* TEST_cceph_messenger_get_conn_by_fd(
        cceph_messenger* messenger, int fd);
extern cceph_connection* TEST_cceph_messenger_get_conn_by_host_and_port(
        cceph_messenger* messenger, const char* host, int port);
#endif
