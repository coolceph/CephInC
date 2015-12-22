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

static int new_connection(int epoll_fd, int socket_fd, int64_t log_id) {
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    int ret;
    
    in_len = sizeof(in_addr);
    infd = accept(socket_fd, &in_addr, &in_len);
    if (infd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        LOG(LL_ERROR, log_id, "accept");
    }
    
    ret = getnameinfo(&in_addr, in_len, 
                      hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                      NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret == 0) {
        LOG(LL_INFO, log_id, "Accepted connection on descriptor %d "
               "(host=%s, port=%s)", infd, hbuf, sbuf);
    }

    ret = make_socket_non_blocking(infd, log_id);
    if (ret == -1) abort();
    
    struct epoll_event event;
    event.data.fd = infd;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, infd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl");
        abort();
    }

    if (ret == -1) return -1;
    else return infd;
}


static msg_header* read_message(int data_fd, int64_t log_id) {
    int8_t op;
    if(read_int8(data_fd, &op, log_id) != 0) return NULL;

    assert(log_id, op == CCEPH_MSG_OP_WRITE);
    msg_write_obj_req* msg = malloc(sizeof(msg_write_obj_req));
    msg->header.op = op;

    if(read_string(data_fd, &(msg->oid_size), &(msg->oid), log_id) != 0) return NULL;
    if(read_int64(data_fd, &(msg->offset), log_id) != 0) return NULL;
    if(read_data(data_fd, &(msg->length), &(msg->data), log_id) != 0) return NULL;
    
    return (msg_header*)msg;
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

static void process_message(msg_header* message, int64_t log_id) {
    assert(log_id, message->op == CCEPH_MSG_OP_WRITE);
    msg_write_obj_req *req = (msg_write_obj_req*)message;
    LOG(LL_INFO, log_id, "req_write, oid: %s, offset: %lu, length: %lu",
           req->oid, req->offset, req->length);

    do_req_write(req);
}

static void new_request(int data_fd, int64_t log_id) {
    msg_header* message = read_message(data_fd, log_id);
    while (message != NULL) {
        LOG(LL_INFO, log_id, "New Message from fd: %d", data_fd);
        process_message(message, log_id);
        message = read_message(data_fd, log_id);
    }
}

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR) 
           || (event.events & EPOLLHUP) 
           || !(event.events & EPOLLIN);
}

static int start_server(char* port, int64_t log_id) {
    int socket_fd, epoll_fd;
    int ret;
    struct epoll_event event;
    struct epoll_event *events;

    socket_fd = create_and_bind(port, log_id);
    if (socket_fd == -1) abort();

    ret = make_socket_non_blocking(socket_fd, log_id);
    if (ret == -1) abort();

    ret = listen(socket_fd, SOMAXCONN);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "listen");
        abort();
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        LOG(LL_ERROR, log_id, "epoll_create");
        abort();
    }

    event.data.fd = socket_fd;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "epoll_ctl");
        abort();
    }

    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof(event));

    /* The event loop */
    while (1) {
        int fd_count = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        int i = 0;
        for (i = 0; i < fd_count; i++) {
            int fd = events[i].data.fd;
            if(is_conn_err(events[i])) {
                LOG(LL_NOTICE, log_id, "Network closed for fd %d.", fd);
                close(fd);
            } else if (socket_fd == fd) {
                int64_t log_id = new_log_id();
                LOG(LL_NOTICE, log_id, "New Connection.");
                new_connection(epoll_fd, fd, log_id);
            } else {
                int64_t log_id = new_log_id();
                LOG(LL_NOTICE, log_id, "New Request from fd %d.", fd);
                new_request(fd, log_id);
            }
        }
    }

    free(events);
    close(socket_fd);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int32_t log_prefix = 201;
    initial_log_id(log_prefix);

    start_server(argv[1], new_log_id());
    return 0;
}

