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

#include "msg/message.h"
#include "msg/msg_write_obj.h"

#define MAXEVENTS 64

static int make_socket_non_blocking(int socket_fd) {
    int flags;

    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
       LOG(LL_ERROR, "fcntl");
       return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1) {
        LOG(LL_ERROR, "fcntl");
        return -1;
    }
    return 0;
}

static int create_and_bind(char *port) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret, socket_fd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    ret = getaddrinfo(NULL, port, &hints, &result);
    if (ret != 0) {
        LOG(LL_ERROR, "getaddrinfo: %s", gai_strerror(ret));
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
        LOG(LL_ERROR, "Could not bind");
        return -1;
    }

    freeaddrinfo(result);
    return socket_fd;
}

static int new_connection(int epoll_fd, int socket_fd) {
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    int ret;
    
    in_len = sizeof(in_addr);
    infd = accept(socket_fd, &in_addr, &in_len);
    if (infd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        LOG(LL_ERROR, "accept");
    }
    
    ret = getnameinfo(&in_addr, in_len, 
                      hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                      NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret == 0) {
        LOG(LL_INFO, "Accepted connection on descriptor %d "
               "(host=%s, port=%s)", infd, hbuf, sbuf);
    }

    ret = make_socket_non_blocking(infd);
    if (ret == -1) abort();
    
    struct epoll_event event;
    event.data.fd = infd;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, infd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, "epoll_ctl");
        abort();
    }

    if (ret == -1) return -1;
    else return infd;
}


static int read_conn(int data_fd, void* buf, size_t size) {
    int total = 0;
    int closed = 0;
    while(size > 0) {
        int count = recv(data_fd, buf, size, 0);
        if (count == -1 && errno != EAGAIN) {
            /* If errno == EAGAIN, that means we have read all
               data. So go back to the main loop. */
            LOG(LL_ERROR, "read close");
            closed = 1;
            break;
        }

        if (count == -1 && errno == EAGAIN && size > 0) {
            LOG(LL_ERROR, "incomplete read, wait and retry to read");
            sleep(1);
            continue;
        }

        if (count == 0) {
            if (size > 0) LOG(LL_ERROR, "incomplete read");
            LOG(LL_ERROR, "read close by ret 0");
            closed = 1;
            break;
        }
        
        buf  += count;
        size -= count;
        total += count;
    }
    if (closed) {
        LOG(LL_INFO, "Closed connection on descriptor %d", data_fd);
        close(data_fd);
    }
    return total;
}

static int read_int8(int data_fd, int8_t* value) {
    return read_conn(data_fd, value, sizeof(*value)) == sizeof(*value) ? 0 : -1;
}

static int read_int64(int data_fd, int64_t* value) {
    return read_conn(data_fd, value, sizeof(*value)) == sizeof(*value) ? 0 : -1;
}

static int read_string(int data_fd, int16_t *size, char **string) {
    if(read_conn(data_fd, size, sizeof(*size)) != sizeof(*size)) {
        return -1;
    }
    
    *string = malloc(*size + 1);
    (*string)[*size] = '\0';
    if (read_conn(data_fd, *string, *size) != *size) {
        free(*string);
        *string = NULL;
        return -1;
    }
    return 0;
}

static int read_data(int data_fd, int64_t *size, char **data) {
    if(read_conn(data_fd, size, sizeof(*size)) != sizeof(*size)) {
        return -1;
    }
    
    *data = malloc(*size);
    if (read_conn(data_fd, *data, *size) != *size) {
        free(*data);
        *data = NULL;
        return -1;
    }
    return 0;
}

static struct msg_header* read_message(int data_fd) {
    int8_t op;
    if(read_int8(data_fd, &op) != 0) return NULL;

    assert(op == CCEPH_MSG_OP_WRITE);
    struct msg_write_obj_req* msg = malloc(sizeof(struct msg_write_obj_req));
    msg->header.op = op;

    if(read_string(data_fd, &(msg->oid_size), &(msg->oid)) != 0) return NULL;
    if(read_int64(data_fd, &(msg->offset)) != 0) return NULL;
    if(read_data(data_fd, &(msg->length), &(msg->data)) != 0) return NULL;
    
    return (struct msg_header*)msg;
}

static void do_req_write(struct msg_write_obj_req* req) {
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

static void process_message(struct msg_header* message) {
    assert(message->op == CCEPH_MSG_OP_WRITE);
    struct msg_write_obj_req *req = (struct msg_write_obj_req*)message;
    LOG(LL_INFO, "req_write, oid: %s, offset: %lu, length: %lu \n",
           req->oid, req->offset, req->length);

    do_req_write(req);
}

static void new_request(int data_fd) {
    struct msg_header* message = read_message(data_fd);
    while (message != NULL) {
        LOG(LL_INFO, "new request at fd: %d\n", data_fd);
        process_message(message);
        message = read_message(data_fd);
    }
}

static int is_conn_err(struct epoll_event event) {
    return (event.events & EPOLLERR) 
           || (event.events & EPOLLHUP) 
           || !(event.events & EPOLLIN);
}

static int start_server(char* port) {
    int socket_fd, epoll_fd;
    int ret;
    struct epoll_event event;
    struct epoll_event *events;

    socket_fd = create_and_bind(port);
    if (socket_fd == -1) abort();

    ret = make_socket_non_blocking(socket_fd);
    if (ret == -1) abort();

    ret = listen(socket_fd, SOMAXCONN);
    if (ret == -1) {
        LOG(LL_ERROR, "listen");
        abort();
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        LOG(LL_ERROR, "epoll_create");
        abort();
    }

    event.data.fd = socket_fd;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, "epoll_ctl");
        abort();
    }

    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof(event));

    /* The event loop */
    while (1) {
        int fd_count = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        int i = 0;
        for (i = 0; i < fd_count; i++) {
            if(is_conn_err(events[i])) {
                close(events[i].data.fd);
            } else if (socket_fd == events[i].data.fd) {
                new_connection(epoll_fd, socket_fd);
            } else {
                new_request(events[i].data.fd);
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

    start_server(argv[1]);
    return 0;
}

