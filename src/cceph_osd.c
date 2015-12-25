/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <inttypes.h>

#include "common/log.h"
#include "common/assert.h"
#include "common/status.h"

#include "message/io.h"
#include "message/messenger.h"
#include "message/msg_header.h"
#include "message/msg_write_obj.h"

#define MAXEVENTS 64

static int make_socket_non_blocking(int socket_fd, int64_t log_id) {
    int flags;

    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
       LOG(LL_ERROR, log_id, "fcntl");
       return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1) {
        LOG(LL_ERROR, log_id, "fcntl");
        return -1;
    }
    return 0;
}

static int create_and_bind(char *port, int64_t log_id) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret, socket_fd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    ret = getaddrinfo(NULL, port, &hints, &result);
    if (ret != 0) {
        LOG(LL_ERROR, log_id, "getaddrinfo: %s", gai_strerror(ret));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1)  continue;

        ret = bind(socket_fd, rp->ai_addr, rp->ai_addrlen);
        if (ret == 0) break;

        close(socket_fd);
    }

    if (rp == NULL) {
        LOG(LL_ERROR, log_id, "Could not bind");
        return -1;
    }

    freeaddrinfo(result);
    return socket_fd;
}

static int new_connection(int socket_fd, msg_handle_t* msg_handle, int64_t log_id) {
    int ret;
    
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    int infd = accept(socket_fd, &in_addr, &in_len);
    if (infd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        LOG(LL_ERROR, log_id, "Accept error.");
        return -1;
    }
    
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    ret = getnameinfo(&in_addr, in_len, 
                      hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                      NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret == 0) {
        LOG(LL_INFO, log_id, "Accepted connection on descriptor %d "
                             "(host=%s, port=%s).", infd, hbuf, sbuf);
    }

    conn_t* conn = new_conn(msg_handle, hbuf, atoi(sbuf), infd, log_id);
    if (conn == NULL)  {
        return -1;
    } else { 
        return 0;
    }
}



static void do_req_write(msg_write_obj_req* req) {
    char data_dir[] = "./data";
    int max_path_length = 4096;

    char path[max_path_length];
    memset(path, '\0', max_path_length);

    strcat(path, data_dir);
    strcat(path, "/");
    strcat(path, req->oid);

    int oid_fd = open(path, O_RDWR | O_CREAT);
    pwrite(oid_fd, req->data, req->length, req->offset);
    close(oid_fd);

    free(req->oid);
    free(req->data);
    free(req);
}

static int process_message(msg_handle_t* msg_handle, conn_t* conn, msg_header* message) {
    int64_t log_id = msg_handle->log_id;

    assert(log_id, message->op == CCEPH_MSG_OP_WRITE);
    msg_write_obj_req *req = (msg_write_obj_req*)message;
    LOG(LL_INFO, log_id, "req_write, oid: %s, offset: %lu, length: %lu",
           req->oid, req->offset, req->length);

    do_req_write(req);
    return 0;
}

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR) 
           || (event.events & EPOLLHUP) 
           || !(event.events & EPOLLIN);
}

static int start_server(char* port, msg_handle_t* msg_handle, int64_t log_id) {
    int ret;

    int socket_fd = create_and_bind(port, log_id);
    if (socket_fd == -1) abort();

    ret = make_socket_non_blocking(socket_fd, log_id);
    if (ret == -1) abort();

    ret = listen(socket_fd, SOMAXCONN);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "listen");
        abort();
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        LOG(LL_ERROR, log_id, "epoll_create");
        abort();
    }

    struct epoll_event event;
    event.data.fd = socket_fd;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl");
        abort();
    }

    while (1) {
        int fd_count = epoll_wait(epoll_fd, &event, 1, -1);
        if (fd_count <= 0) {
            //TODO: error log here
            continue;
        }

        int fd = event.data.fd;
        assert(log_id, fd == socket_fd);
        if (is_conn_err(event)) {
            LOG(LL_FATAL, log_id, "Listen Socked Closed.");
            close(fd);
            abort();
        }

        new_connection(socket_fd, msg_handle, log_id);
    }

    close(socket_fd);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char* port = argv[1];

    int32_t log_prefix = 201;
    initial_log_id(log_prefix);

    msg_handle_t* msg_handle = start_messager(&process_message, new_log_id());
    start_server(port, msg_handle, new_log_id());
    return 0;
}

