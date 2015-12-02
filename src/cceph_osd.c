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

#include "common/log.h"

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

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    ret = getaddrinfo(NULL, port, &hints, &result);
    if (ret != 0) {
        LOG(LL_ERROR, "getaddrinfo: %s\n", gai_strerror(ret));
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
        LOG(LL_ERROR, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(result);
    return socket_fd;
}

static int new_connection(int base_fd, int new_fd) {
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    int ret;
    
    in_len = sizeof in_addr;
    infd = accept(new_fd, &in_addr, &in_len);
    if (infd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        LOG(LL_ERROR, "accept");
    }
    
    ret = getnameinfo(&in_addr, in_len, 
                      hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                      NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret == 0) {
        LOG(LL_INFO, "Accepted connection on descriptor %d "
               "(host=%s, port=%s)\n", infd, hbuf, sbuf);
    }

    ret = make_socket_non_blocking(infd);
    if (ret == -1) abort();
    
    struct epoll_event event;
    event.data.fd = infd;
    event.events = EPOLLIN | EPOLLET;
    ret = epoll_ctl(base_fd, EPOLL_CTL_ADD, infd, &event);
    if (ret == -1) {
        LOG(LL_ERROR, "epoll_ctl");
        abort();
    }
}

static int new_reqeust(int data_fd) {
    int closed = 0;
    int ret = 0;
    
    while(1) {
        ssize_t count;
        char buf[512];
    
        count = read(data_fd, buf, sizeof(buf));
        if (count == -1 && errno != EAGAIN) {
            /* If errno == EAGAIN, that means we have read all
               data. So go back to the main loop. */
            LOG(LL_ERROR, "read");
            closed = 1;
            break;
        }

        if (count == -1) {
            break;
        }

        if (count == 0) {
            closed = 1;
            break;
        }
    
        ret = write(1, buf, count);
        if (ret == -1) {
            LOG(LL_ERROR, "write");
            abort();
        }
    }
    
    if (closed) {
        LOG(LL_INFO, "Closed connection on descriptor %d\n", data_fd);
        close(data_fd);
    }
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
    events = calloc(MAXEVENTS, sizeof event);

    /* The event loop */
    while (1) {
        int fd_count = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        int i = 0;
        for (i = 0; i < fd_count; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
              /* An error has occured on this fd, or the socket is not
 *                  ready for reading (why were we notified then?) */
                perror("epoll error\n");
                close(events[i].data.fd);
            } else if (socket_fd == events[i].data.fd) {
                new_connection(epoll_fd, socket_fd);
            } else {
                new_reqeust(events[i].data.fd);
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

