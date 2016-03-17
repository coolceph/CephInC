#include "message/messenger.h"

#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"

#include "message/io.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

//caller must has messenger->conn_list_lock
static cceph_connection* cceph_messenger_get_conn_by_id(
        cceph_messenger* messenger, int id) {
    cceph_list_head *pos;
    cceph_connection *conn = NULL;
    cceph_connection *result = NULL;

    cceph_list_for_each(pos, &(messenger->conn_list.list_node)) {
        conn = cceph_list_entry(pos, cceph_connection, list_node);
        if (conn->id == id) {
            result = conn;
            break;
        }
    }
    return result;
}
//caller must has messenger->conn_list_lock
static cceph_connection* cceph_messenger_get_conn_by_fd(
        cceph_messenger* messenger, int fd) {
    cceph_list_head *pos;
    cceph_connection *conn = NULL;
    cceph_connection *result = NULL;

    cceph_list_for_each(pos, &(messenger->conn_list.list_node)) {
        conn = cceph_list_entry(pos, cceph_connection, list_node);
        if (conn->fd == fd) {
            result = conn;
            break;
        }
    }
    return result;
}
//caller must has messenger->conn_list_lock
static cceph_connection* cceph_messenger_get_conn_by_host_and_port(
        cceph_messenger* messenger, const char* host, int port) {
    cceph_list_head *pos;
    cceph_connection *conn = NULL;
    cceph_connection *result = NULL;

    cceph_list_for_each(pos, &(messenger->conn_list.list_node)) {
        conn = cceph_list_entry(pos, cceph_connection, list_node);
        if (conn->port == port && strcmp(conn->host, host) == 0) {
            result = conn;
            break;
        }
    }
    return result;
}

extern int cceph_messenger_close_conn(
        cceph_messenger* messenger, cceph_conn_id_t id, int64_t log_id) {
    pthread_rwlock_wrlock(&messenger->conn_list_lock);
    cceph_connection* conn = cceph_messenger_get_conn_by_id(messenger, id);
    if (conn == NULL) {
        pthread_rwlock_unlock(&messenger->conn_list_lock);
        LOG(LL_NOTICE, log_id, "The conn %ld is not found when close.", id);
        return CCEPH_ERR_CONN_NOT_FOUND;
    }

    cceph_list_delete(&conn->list_node);
    pthread_rwlock_unlock(&messenger->conn_list_lock);

    LOG(LL_NOTICE, log_id, "Close conn %s:%d, conn_id %ld, fd %d.",
            conn->host, conn->port, conn->id, conn->fd);

    pthread_mutex_lock(&conn->lock);
    close(conn->fd);
    free(conn->host); conn->host = NULL;
    pthread_mutex_unlock(&conn->lock);

    pthread_mutex_destroy(&conn->lock);
    free(conn); conn = NULL;
    return 0;
}

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR)
           || (event.events & EPOLLHUP)
           || !(event.events & EPOLLIN);
}

static cceph_msg_header* cceph_messenger_read_msg(
        cceph_messenger *messenger, cceph_conn_id_t conn_id, int fd, int64_t log_id) {
    LOG(LL_INFO, log_id, "Read Message from conn_id %ld, fd %d.", conn_id, fd);

    //Read msg_hedaer
    cceph_msg_header header;
    int ret = cceph_msg_header_recv(fd, &header, log_id);
    if (ret == CCEPH_ERR_CONN_CLOSED) {
        LOG(LL_NOTICE, log_id, "Read msg_header from conn_id %ld failed, conn closed.",
                conn_id);
        cceph_messenger_close_conn(messenger, conn_id, log_id);
        return NULL;
    } else if (ret != 0) {
        LOG(LL_ERROR, log_id, "Read msg_header from conn_id %ld error %d(%s).",
                conn_id, ret, errno_str(ret));
        return NULL;
    } else {
        LOG(LL_INFO, log_id, "read msg_header form conn_id %ld, op %s(%d), log_id %ld.",
            conn_id, cceph_str_msg_op(header.op), header.op, header.log_id);
    }

    //Read message
    cceph_msg_header *message = NULL;
    switch (header.op) {
        case CCEPH_MSG_OP_WRITE:
            {
                cceph_msg_write_obj_req *msg = cceph_msg_write_obj_req_new(log_id);
                ret = cceph_msg_write_obj_req_recv(fd, msg, log_id);
                if (ret != 0) cceph_msg_write_obj_req_free(&msg, log_id);
                message = (cceph_msg_header*)msg;
                break;
            }
        case CCEPH_MSG_OP_WRITE_ACK:
            {
                cceph_msg_write_obj_ack *msg =  cceph_msg_write_obj_ack_new(log_id);
                ret = cceph_msg_write_obj_ack_recv(fd, msg, log_id);
                if (ret != 0) cceph_msg_write_obj_ack_free(&msg, log_id);
                message = (cceph_msg_header*)msg;
                break;
            }
        case CCEPH_MSG_OP_READ:
            assert(log_id, "Not Impl" != 0);
            break;
        case CCEPH_MSG_OP_READ_ACK:
            assert(log_id, "Not Impl" != 0);
            break;
        default:
            ret = CCEPH_ERR_UNKNOWN_OP;
    }

    if (ret == CCEPH_ERR_CONN_CLOSED) {
        LOG(LL_NOTICE, log_id, "Read message from conn_id %ld error, conn closed.",
                conn_id);
        cceph_messenger_close_conn(messenger, conn_id, log_id);
        return NULL;
    } else if (ret != 0) {
        LOG(LL_ERROR, log_id, "Read message from conn_id %ld error %d(%s).",
                conn_id, ret, errno_str(ret));
        return NULL;
    } else {
        LOG(LL_INFO, log_id, "Read message form conn_id %ld, op %s(%d), log_id %ld.",
            conn_id, cceph_str_msg_op(header.op), header.op, header.log_id);
    }

    message->op = header.op;
    message->log_id = header.log_id;
    return message;
}
static int write_message(cceph_connection* conn, cceph_msg_header* msg, int64_t log_id) {
    int fd = conn->fd;
    cceph_conn_id_t conn_id = conn->id;

    LOG(LL_INFO, log_id, "Write Message to conn_id %ld, fd %d, op %s(%d).",
        conn_id, fd, cceph_str_msg_op(msg->op), msg->op);

    //Write msg_hedaer
    int ret = cceph_msg_header_send(fd, msg, log_id);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Write msg_header to conn_id %ld error, errno %d(%s).",
                conn_id, ret, errno_str(ret));
        return ret;
    } else {
        LOG(LL_INFO, log_id, "Write msg_header to conn_id %ld.", conn_id);
    }

    //Read message
    switch (msg->op) {
        case CCEPH_MSG_OP_WRITE:
            {
                ret = cceph_msg_write_obj_req_send(fd, (cceph_msg_write_obj_req*)msg, log_id);
                break;
            }
        case CCEPH_MSG_OP_WRITE_ACK:
            {
                ret = cceph_msg_write_obj_ack_send(fd, (cceph_msg_write_obj_ack*)msg, log_id);
                break;
            }
        case CCEPH_MSG_OP_READ:
            assert(log_id, "Not Impl" != 0);
            break;
        case CCEPH_MSG_OP_READ_ACK:
            assert(log_id, "Not Impl" != 0);
            break;
        default:
            ret = CCEPH_ERR_UNKNOWN_OP;
    }

    if (ret != 0) {
        LOG(LL_ERROR, log_id, "Write message to conn_id %ld error, errno %d(%s).",
                conn_id, ret, errno_str(ret));
        return ret;
    } else {
        LOG(LL_INFO, log_id, "Write message to conn_id %ld success.", conn_id);
    }

    return 0;
}
static int wait_for_next_msg(cceph_messenger* messenger, int fd) {
    struct epoll_event ctl_event;
    ctl_event.data.fd = fd;
    ctl_event.events = EPOLLIN | EPOLLONESHOT;
    return epoll_ctl(messenger->epoll_fd, EPOLL_CTL_MOD, fd, &ctl_event);
}

static void* start_epoll(void* arg) {
    cceph_messenger* messenger = (cceph_messenger*)arg;
    int64_t log_id = messenger->log_id;
    assert(log_id, messenger->epoll_fd != -1);

    struct epoll_event event;
    bzero(&event, sizeof(event));
    while (1) {
        int fd_count = epoll_wait(messenger->epoll_fd, &event, 1, -1);
        if (fd_count <= 0) {
            if (errno  == EINTR) {
                continue;
            } else {
                LOG(LL_ERROR, log_id, "epoll_wait error, errno %d.", fd_count);
                continue;
            }
        }

        int fd = event.data.fd;
        cceph_conn_id_t conn_id = -1;

        pthread_rwlock_rdlock(&messenger->conn_list_lock);
        cceph_connection* conn = cceph_messenger_get_conn_by_fd(messenger, fd);
        if (conn == NULL) {
            LOG(LL_ERROR, log_id, "epoll_wait return fd %d, but conn is not found.", fd);
            pthread_rwlock_unlock(&messenger->conn_list_lock);
            continue;
        } else {
            LOG(LL_INFO, log_id, "Data received from conn %s:%d, conn_id %ld, fd %d.",
                    conn->host, conn->port, conn->id, fd);
            conn_id = conn->id;
            conn = NULL; //conn can NOT be used with conn_list_lock or conn->lock, it may be freed.
            pthread_rwlock_unlock(&messenger->conn_list_lock);
        }

        if (is_conn_err(event)) {
            LOG(LL_INFO, log_id, "Network closed for conn_id %ld, fd %d.", conn_id, fd);
            cceph_messenger_close_conn(messenger, conn_id, log_id);
            continue;
        }

        if (fd == messenger->wake_thread_pipe_fd[0]) {
            LOG(LL_INFO, log_id, "thread is wake up by wake_up_pipe, thread_id %lu.", pthread_self());
            cceph_messenger_op_t op;
            int ret = read(fd, &op, sizeof(op));
            assert(log_id, ret == sizeof(op));
            assert(log_id, op == CCEPH_MESSENGER_OP_STOP); //TODO: there maybe other op in the feature

            ret = wait_for_next_msg(messenger, fd);
            assert(log_id, ret == 0);
            break;
        }

        log_id = cceph_log_new_id(); //new message, new log_id, just for read process
        cceph_msg_header* msg = cceph_messenger_read_msg(messenger, conn_id, fd, log_id);
        if (msg == NULL) {
            LOG(LL_NOTICE, log_id, "Read message from conn_id %ld, fd %d failed, conn may closed.", conn_id, fd);
            continue;
        } else {
            LOG(LL_INFO, log_id, "Msg read from conn_id %ld, fd %d, op %s(%d), log_id %ld.",
                conn_id, fd, cceph_str_msg_op(msg->op), msg->op, msg->log_id);
        }

        //Wait for the next msg
        int ret = wait_for_next_msg(messenger, fd);
        if (ret < -1) {
            LOG(LL_ERROR, log_id, "wait for next msg of conn_id %ld, fd %d error, errno %d(%s).",
                    conn_id, fd, ret, errno_str(ret));
            cceph_messenger_close_conn(messenger, conn_id, log_id);
        } else {
            LOG(LL_INFO, log_id, "wait for new msg of conn_id %ld, fd %d.", conn_id, fd);
        }

        log_id = messenger->log_id; //reset log_id back to messenger->log_id

        //Process the msg
        //the log id will not be passed, while the msg->log_id will be used
        messenger->msg_process(messenger, conn_id, msg, messenger->context);
    }

    return NULL;
}

extern cceph_messenger* cceph_messenger_new(
        cceph_msg_messengerr msg_messengerr, void* context, int64_t log_id) {
    cceph_messenger* messenger = (cceph_messenger*)malloc(sizeof(cceph_messenger));
    messenger->epoll_fd = -1;
    messenger->state = CCEPH_MESSENGER_STATE_UNKNOWN;
    messenger->log_id = log_id;
    messenger->msg_process = msg_messengerr;
    messenger->context = context;
    messenger->thread_count = 2; //TODO: we need a opinion
    messenger->thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * messenger->thread_count);
    cceph_atomic_set64(&messenger->next_conn_id, 1);

    cceph_list_head_init(&messenger->conn_list.list_node);
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&messenger->conn_list_lock, &attr);

    //initial wake_thread_pipe;
    int ret = pipe(messenger->wake_thread_pipe_fd);
    if (ret < 0) {
        LOG(LL_FATAL, log_id, "can't initial cceph_messenger->wake_thread_pipe, errno %d.", ret);
        free(messenger->thread_ids); messenger->thread_ids = NULL;
        free(messenger); messenger = NULL;
        return NULL;
    }

    //create epoll_fd
    messenger->epoll_fd = epoll_create1(0);
    if (messenger->epoll_fd == -1) {
        LOG(LL_FATAL, log_id, "epoll_create for cceph_messenger error, errno: %d.", errno);
        close(messenger->wake_thread_pipe_fd[0]);
        close(messenger->wake_thread_pipe_fd[1]);
        free(messenger->thread_ids); messenger->thread_ids = NULL;
        free(messenger); messenger = NULL;
        return NULL;
    }

    return messenger;
}

extern int cceph_messenger_start(cceph_messenger* messenger, int64_t log_id) {

    assert(log_id, messenger->state == CCEPH_MESSENGER_STATE_UNKNOWN);
    messenger->state = CCEPH_MESSENGER_STATE_NORMAL;

    //add the wake_thread_pipe_fd to epoll set
    int ret = cceph_messenger_add_conn(messenger, "wake_thread_pipe", 0, messenger->wake_thread_pipe_fd[0], log_id);
    if (ret < 0) {
        LOG(LL_FATAL, log_id, "Add wake_thread_pipe to cceph_messenger, errno %d(%s).", ret, errno_str(ret));
        messenger->state = CCEPH_MESSENGER_STATE_UNKNOWN;
        return ret;
    }

    //run start_epoll by thread pool
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int i = 0;
    for (i = 0; i < messenger->thread_count; i++) {
        ret = pthread_create(messenger->thread_ids + i, &thread_attr, &start_epoll, messenger);
        if (ret < 0) {
            LOG(LL_FATAL, log_id, "create start_epoll thread error, errno %d.", ret);
            messenger->state = CCEPH_MESSENGER_STATE_UNKNOWN;
            return ret;
        }
    }

    return 0;
}
extern int cceph_messenger_stop(cceph_messenger* messenger, int64_t log_id) {
    assert(log_id, messenger->state == CCEPH_MESSENGER_STATE_NORMAL);

    int i = 0, ret = 0;
    cceph_messenger_op_t op = CCEPH_MESSENGER_OP_STOP;
    for (i = 0; i < messenger->thread_count; i++) {
        ret = write(messenger->wake_thread_pipe_fd[1], &op, sizeof(op));
        assert(log_id, ret == sizeof(op));
    }
    for (i = 0; i < messenger->thread_count; i++) {
        ret = pthread_join(*(messenger->thread_ids + i), NULL);
        assert(log_id, ret == 0);
    }

    messenger->state = CCEPH_MESSENGER_STATE_DESTORY;
    return 0;
}
extern int cceph_messenger_free(cceph_messenger** messenger, int64_t log_id) {
    assert(log_id, *messenger != NULL);
    assert(log_id, (*messenger)->thread_ids != NULL);
    assert(log_id, (*messenger)->state == CCEPH_MESSENGER_STATE_DESTORY);

    free((*messenger)->thread_ids);
    (*messenger)->thread_ids = NULL;

    free(*messenger);
    *messenger = NULL;
    return 0;
}

extern cceph_conn_id_t cceph_messenger_add_conn(
        cceph_messenger* messenger, const char* host, int port, int fd, int64_t log_id) {
    assert(log_id, messenger != NULL);
    assert(log_id, messenger->state == CCEPH_MESSENGER_STATE_NORMAL);
    assert(log_id, host != NULL);

    //New connection from params
    cceph_connection* conn = (cceph_connection*)malloc(sizeof(cceph_connection));
    conn->id = cceph_atomic_add64(&messenger->next_conn_id, 1);
    conn->fd = fd;
    conn->port = port;
    conn->host = (char*)malloc(sizeof(char) * strlen(host));
    strcpy(conn->host, host);
    pthread_mutex_init(&conn->lock, NULL);

    //Add conn to messenger->conn_list
    pthread_rwlock_wrlock(&messenger->conn_list_lock);
    cceph_list_add(&conn->list_node, &messenger->conn_list.list_node);
    pthread_rwlock_unlock(&messenger->conn_list_lock);

    //Add fd to epoll set
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT;
    int ret = epoll_ctl(messenger->epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret < -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl for new conn %s:%d, fd %d error, errno %d(%s).",
                host, port, fd, ret, errno_str(ret));
        cceph_messenger_close_conn(messenger, conn->id, log_id);
        return -1;
    }

    LOG(LL_NOTICE, log_id, "New conn %s:%d, fd %d.", host, port, fd);
    return conn->id;
}
extern cceph_conn_id_t cceph_messenger_get_conn(
        cceph_messenger* messenger, const char* host, int port, int64_t log_id) {
    assert(log_id, messenger != NULL);
    assert(log_id, messenger->state == CCEPH_MESSENGER_STATE_NORMAL);
    assert(log_id, host != NULL);
    assert(log_id, port > 0);

    cceph_conn_id_t conn_id = -1;

    //try to find conn from list first;
    pthread_rwlock_rdlock(&messenger->conn_list_lock);
    cceph_connection* conn = cceph_messenger_get_conn_by_host_and_port(messenger, host, port);
    if (conn != NULL) {
        conn_id = conn->id;
    }
    conn = NULL;
    pthread_rwlock_unlock(&messenger->conn_list_lock);

    if (conn_id != -1) {
        LOG(LL_DEBUG, log_id, "Conn for %s:%d found in current conn_list, return conn_id.", host, port);
        return conn_id;
    }
    LOG(LL_DEBUG, log_id, "Conn for %s:%d is not found in current conn_list, try to connect.", host, port);

    struct sockaddr_in server_addr_in;
    bzero(&server_addr_in, sizeof(server_addr_in) );
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr_in.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(fd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));
    if (ret < 0) {
        LOG(LL_ERROR, log_id, "Connect to conn %s:%d error, errno %d.", host, port, ret);
        return ret;
    }
    LOG(LL_DEBUG, log_id, "Connect to conn %s:%d success.", host, port);

    conn_id = cceph_messenger_add_conn(messenger, host, port, fd, log_id);
    assert(log_id, conn_id > 0);

    return conn_id;
}

extern int cceph_messenger_send_msg(
        cceph_messenger* messenger, cceph_conn_id_t conn_id, cceph_msg_header* msg, int64_t log_id) {
    assert(log_id, messenger != NULL);
    assert(log_id, messenger->state == CCEPH_MESSENGER_STATE_NORMAL);
    assert(log_id, msg != NULL);

    pthread_rwlock_rdlock(&messenger->conn_list_lock);
    cceph_connection* conn = cceph_messenger_get_conn_by_id(messenger, conn_id);
    if (conn == NULL) {
        pthread_rwlock_unlock(&messenger->conn_list_lock);
        LOG(LL_ERROR, log_id, "cceph_messenger_send_msg can't find conn_id %ld.", conn_id);
        return CCEPH_ERR_CONN_NOT_FOUND;
    }

    pthread_mutex_lock(&conn->lock);
    pthread_rwlock_unlock(&messenger->conn_list_lock);

    if (conn->state == CCEPH_CONN_STATE_CLOSED) {
        pthread_mutex_unlock(&conn->lock);
        LOG(LL_ERROR, log_id, "Conn %ld has already closed.", conn_id);
        return CCEPH_ERR_CONN_CLOSED;
    }

    LOG(LL_INFO, log_id, "Send msg to %s:%d, conn_id %ld, fd %d, op %s(%d).",
        conn->host, conn->port, conn->id, conn->fd, cceph_str_msg_op(msg->op), msg->op);
    if (write_message(conn, msg, log_id) != 0) {
        conn->state = CCEPH_CONN_STATE_CLOSED;
        pthread_mutex_unlock(&conn->lock);

        LOG(LL_ERROR, log_id, "Write message to conn_id %ld failed, close it.", conn_id);
        cceph_messenger_close_conn(messenger, conn_id, log_id);
        return CCEPH_ERR_WRITE_CONN_ERR;
    }

    pthread_mutex_unlock(&conn->lock);
    return 0;
}

extern cceph_connection* TEST_cceph_messenger_get_conn_by_id(
        cceph_messenger* messenger, int id) {
    return cceph_messenger_get_conn_by_id(messenger, id);
}
extern cceph_connection* TEST_cceph_messenger_get_conn_by_fd(
        cceph_messenger* messenger, int fd) {
    return cceph_messenger_get_conn_by_fd(messenger, fd);
}
extern cceph_connection* TEST_cceph_messenger_get_conn_by_host_and_port(
        cceph_messenger* messenger, const char* host, int port) {
    return cceph_messenger_get_conn_by_host_and_port(messenger, host, port);
}
