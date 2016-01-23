#include "message/messenger.h"

#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "include/errno.h"

#include "common/assert.h"
#include "common/log.h"

#include "message/io.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

//caller must has handle->conn_list_lock
static connection* get_conn_by_id(msg_handle* handle, int id) {
    struct list_head *pos;
    connection *conn = NULL;
    connection *result = NULL;

    list_for_each(pos, &(handle->conn_list.list_node)) {
        conn = list_entry(pos, connection, list_node);
        if (conn->id == id) {
            result = conn;
            break;
        }
    }
    return result;
}
//caller must has handle->conn_list_lock
static connection* get_conn_by_fd(msg_handle* handle, int fd) {
    struct list_head *pos;
    connection *conn = NULL;
    connection *result = NULL;

    list_for_each(pos, &(handle->conn_list.list_node)) {
        conn = list_entry(pos, connection, list_node);
        if (conn->fd == fd) {
            result = conn;
            break;
        }
    }
    return result;
}
//caller must has handle->conn_list_lock
static connection* get_conn_by_host_and_port(msg_handle* handle, char* host, int port) {
    struct list_head *pos;
    connection *conn = NULL;
    connection *result = NULL;

    list_for_each(pos, &(handle->conn_list.list_node)) {
        conn = list_entry(pos, connection, list_node);
        if (conn->port == port && strcmp(conn->host, host) == 0) {
            result = conn;
            break;
        }
    }
    return result;
}

extern int close_conn(msg_handle* handle, conn_id_t id, int64_t log_id) {
    pthread_rwlock_wrlock(&handle->conn_list_lock);
    connection* conn = get_conn_by_id(handle, id);
    if (conn == NULL) {
        pthread_rwlock_unlock(&handle->conn_list_lock);
        LOG(LL_NOTICE, log_id, "The conn %ld is not found when close", id);
        return CCEPH_ERR_CONN_NOT_FOUND;
    }

    list_del(&conn->list_node);
    pthread_rwlock_unlock(&handle->conn_list_lock);

    LOG(LL_NOTICE, log_id, "Close conn %s:%d, conn_id %ld, fd %d", conn->host, conn->port, conn->id, conn->fd);

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

static msg_header* read_message(msg_handle *handle, conn_id_t conn_id, int fd, int64_t log_id) {
    LOG(LL_INFO, log_id, "Read Message from conn_id %ld, fd %d.", conn_id, fd);

    //Read msg_hedaer
    msg_header header;
    int ret = recv_msg_header(fd, &header, log_id);
    if (ret == CCEPH_ERR_CONN_CLOSED) {
        LOG(LL_NOTICE, log_id, "Read msg_header from conn_id %ld error, conn closed", conn_id);
        close_conn(handle, conn_id, log_id);
        return NULL;
    } else if (ret != 0) {
        LOG(LL_ERROR, log_id, "Read msg_header from conn_id %ld error %d.", conn_id, ret);
        return NULL;
    } else {
        LOG(LL_INFO, log_id, "read msg_header form conn_id %ld, op %d, log_id %ld", conn_id, header.op, header.log_id);
    }

    //Read message
    msg_header *message = NULL;
    switch (header.op) {
        case CCEPH_MSG_OP_WRITE:
            {
                msg_write_obj_req *msg = malloc_msg_write_obj_req(log_id);
                ret = recv_msg_write_obj_req(fd, msg, log_id);
                if (ret != 0) free_msg_write_obj_req(&msg, log_id);
                message = (msg_header*)msg;
                break;
            }
        case CCEPH_MSG_OP_WRITE_ACK:
            {
                msg_write_obj_ack *msg =  malloc_msg_write_obj_ack(log_id);
                ret = recv_msg_write_obj_ack(fd, msg, log_id);
                if (ret != 0) free_msg_write_obj_ack(&msg, log_id);
                message = (msg_header*)msg;
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
        LOG(LL_NOTICE, log_id, "Read message from conn_id %ld error, conn closed", conn_id);
        close_conn(handle, conn_id, log_id);
        return NULL;
    } else if (ret != 0) {
        LOG(LL_ERROR, log_id, "Read message from conn_id %ld error %d.", conn_id, ret);
        return NULL;
    } else {
        LOG(LL_INFO, log_id, "Read message form conn_id %ld, op %d, log_id %ld", conn_id, header.op, header.log_id);
    }

    message->op = header.op;
    message->log_id = header.log_id;
    return message;
}
static int write_message(msg_handle* handle, connection* conn, msg_header* msg, int64_t log_id) {
    return 0;
}

static void* start_epoll(void* arg) {
    msg_handle* handle = (msg_handle*)arg;
    int64_t log_id = handle->log_id;
    assert(log_id, handle->epoll_fd != -1);

    struct epoll_event event;
    bzero(&event, sizeof(event));
    while (1) {
        int fd_count = epoll_wait(handle->epoll_fd, &event, 1, -1);
        if (fd_count <= 0) {
            if (errno  == EINTR) {
                continue;
            } else {
                perror("epoll_wait");
                LOG(LL_ERROR, log_id, "epoll_wait return: %d", fd_count);
                continue;
            }
        }

        int fd = event.data.fd;
        conn_id_t conn_id = -1;

        pthread_rwlock_rdlock(&handle->conn_list_lock);
        connection* conn = get_conn_by_fd(handle, fd);
        if (conn == NULL) {
            LOG(LL_ERROR, log_id, "epoll_wait return fd %d, but conn is not found", fd);
            pthread_rwlock_unlock(&handle->conn_list_lock);
            continue;
        } else {
            LOG(LL_INFO, log_id, "Data received from conn %s:%d, conn_id %ld, fd %d", conn->host, conn->port, conn->id, fd);
            conn_id = conn->id;
            conn = NULL; //conn can NOT be used with conn_list_lock or conn->lock, it may be freed.
            pthread_rwlock_unlock(&handle->conn_list_lock);
        }

        if (is_conn_err(event)) {
            LOG(LL_INFO, log_id, "Network closed for conn %ld, fd %d.", conn_id, fd);
            close_conn(handle, conn_id, log_id);
            continue;
        }

        if (fd == handle->wake_thread_pipe_fd[1]) {
            LOG(LL_INFO, log_id, "thread is wake up by wake_up_pipe, thread_id: %lu", pthread_self());
            //TODO: stop messenger
        }

        log_id = new_log_id(); //new message, new log_id, just for read process
        msg_header* msg = read_message(handle, conn_id, fd, log_id);
        if (msg == NULL) {
            LOG(LL_ERROR, log_id, "Read message from conn %ld, fd %d error.", conn_id, fd);
            continue;
        } else {
            LOG(LL_INFO, log_id, "Msg read from conn %ld, fd %d, op %d, log_id %ld", conn_id, fd, msg->op, msg->log_id);
        }

        //Wait for the next msg
        struct epoll_event ctl_event;
        ctl_event.data.fd = fd;
        ctl_event.events = EPOLLIN | EPOLLONESHOT;
        int ret = epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, fd, &ctl_event);
        if (ret < -1) {
            LOG(LL_ERROR, log_id, "epoll_ctl for conn %ld, fd %d, error: %d", conn_id, fd, ret);
            close_conn(handle, conn_id, log_id);
        } else {
            LOG(LL_INFO, log_id, "epoll_ctl for conn %ld, fd %d, wait for next msg", conn_id, fd);
        }

        log_id = handle->log_id; //reset log_id back to handle->log_id

        //Process the msg
        //the log id will not be passed, while the msg->log_id will be used
        handle->msg_process(handle, conn_id, msg);
    }

    return NULL;
}

static msg_handle* new_msg_handle(msg_handler msg_handler, int64_t log_id) {
    msg_handle* handle = (msg_handle*)malloc(sizeof(msg_handle));
    handle->epoll_fd = -1;
    handle->log_id = log_id;
    handle->msg_process = msg_handler;
    handle->thread_count = 2; //TODO: we need a opinion
    handle->thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * handle->thread_count);
    atomic_set64(&handle->next_conn_id, 1);

    init_list_head(&handle->conn_list.list_node);
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&handle->conn_list_lock, &attr);

    //initial wake_thread_pipe;
    int ret = pipe(handle->wake_thread_pipe_fd);
    if (ret < 0) {
        LOG(LL_FATAL, log_id, "can't initial msg_handle->wake_thread_pipe, error: %d", ret);
        free(handle->thread_ids); handle->thread_ids = NULL;
        free(handle); handle = NULL;
        return NULL;
    }
    
    //create epoll_fd
    handle->epoll_fd = epoll_create1(0);
    if (handle->epoll_fd == -1) {
        LOG(LL_FATAL, log_id, "epoll_create for msg_handle error, errno: %d", errno);
        close(handle->wake_thread_pipe_fd[0]);
        close(handle->wake_thread_pipe_fd[1]);
        free(handle->thread_ids); handle->thread_ids = NULL;
        free(handle); handle = NULL;
        return NULL;
    }

    return handle;
}

extern msg_handle* start_messager(msg_handler msg_handler, int64_t log_id) {

    msg_handle* handle = new_msg_handle(msg_handler, log_id);
    if (handle == NULL) {
        LOG(LL_FATAL, log_id, "new_msg_handle failed");
        return NULL;
    }

    //add the wake_thread_pipe_fd to epoll set
    new_conn(handle, "wake_thread_pipe", 0, handle->wake_thread_pipe_fd[1], log_id);

    //run start_epoll by thread pool
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int i = 0; for (i = 0; i < handle->thread_count; i++) {
        int ret = pthread_create(handle->thread_ids + i, &thread_attr, &start_epoll, handle);
        if (ret < 0) {
            LOG(LL_FATAL, log_id, "create start_epoll thread error: %d", ret);
            abort();
        }
    }
    
    return handle;
}

extern conn_id_t new_conn(msg_handle* handle, char* host, int port, int fd, int64_t log_id) {
    //New connection from params
    connection* conn = (connection*)malloc(sizeof(connection));
    conn->id = atomic_add64(&handle->next_conn_id, 1);
    conn->fd = fd;
    conn->port = port;
    conn->host = (char*)malloc(sizeof(char) * strlen(host));
    strcpy(conn->host, host);
    pthread_mutex_init(&conn->lock, NULL);

    //Add conn to handle->conn_list
    pthread_rwlock_wrlock(&handle->conn_list_lock);
    list_add(&conn->list_node, &handle->conn_list.list_node);
    pthread_rwlock_unlock(&handle->conn_list_lock);

    //Add fd to epoll set
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT;
    int ret = epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret < -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl for new conn %s:%d, fd %d, error: %d", host, port, fd, ret);
        close_conn(handle, conn->id, log_id);
        return -1;
    }

    LOG(LL_NOTICE, log_id, "New conn %s:%d, fd %d", host, port, fd);
    return conn->id;
}
extern conn_id_t get_conn(msg_handle* handle, char* host, int port, int64_t log_id) {
/*
 *    struct sockaddr_in server_addr_in;
 *    bzero(&server_addr_in, sizeof(server_addr_in) );
 *    server_addr_in.sin_family = AF_INET;
 *    server_addr_in.sin_port = htons(port);
 *    inet_pton(AF_INET, host, &server_addr_in.sin_addr);
 *
 *    int fd = socket(AF_INET, SOCK_STREAM, 0);
 *    int ret = connect(fd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));
 *    if (ret < 0) {
 *        LOG(LL_ERROR, log_id, "connect to %s:%d error: %d", host, port, ret);
 *        return ret;
 *    }
 */
    return 0;
}

extern int send_msg(msg_handle* handle, conn_id_t conn_id, msg_header* msg, int64_t log_id) {
    assert(log_id, msg != NULL);
    assert(log_id, handle != NULL);

    pthread_rwlock_rdlock(&handle->conn_list_lock);
    connection* conn = get_conn_by_id(handle, conn_id);
    if (conn == NULL) {
        pthread_rwlock_unlock(&handle->conn_list_lock);
        LOG(LL_ERROR, log_id, "send_msg can't find conn_id %ld.", conn_id);
        return CCEPH_ERR_CONN_NOT_FOUND;
    }

    pthread_mutex_lock(&conn->lock);
    pthread_rwlock_unlock(&handle->conn_list_lock);

    if (conn->state == CCEPH_CONN_STATE_CLOSED) {
        pthread_mutex_unlock(&conn->lock);
        LOG(LL_ERROR, log_id, "Conn %ld has already closed.", conn_id);
        return CCEPH_ERR_CONN_CLOSED;
    }

    LOG(LL_INFO, log_id, "Send msg to %s:%d, conn_id %ld, fd %d, op %d.",
                         conn->host, conn->port, conn->id, conn->fd, msg->op);
    if (write_message(handle, conn, msg, log_id) != 0) {
        conn->state = CCEPH_CONN_STATE_CLOSED;
        pthread_mutex_unlock(&conn->lock);

        LOG(LL_ERROR, log_id, "Write message to conn_id %ld failed, close it.", conn_id);
        close_conn(handle, conn_id, log_id);
        return CCEPH_ERR_WRITE_CONN_ERR;
    }

    pthread_mutex_unlock(&conn->lock);
    return 0;
}

extern msg_handle* TEST_new_msg_handle(msg_handler msg_handler, int64_t log_id) {
    return new_msg_handle(msg_handler, log_id);
}

extern connection* TEST_get_conn_by_id(msg_handle* handle, int id) {
    return get_conn_by_id(handle, id);
}
extern connection* TEST_get_conn_by_fd(msg_handle* handle, int fd) {
    return get_conn_by_fd(handle, fd);
}
extern connection* TEST_get_conn_by_host_and_port(msg_handle* handle, char* host, int port) {
    return get_conn_by_host_and_port(handle, host, port);
}
