#include "message/server_messenger.h"

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <inttypes.h>

#include "common/assert.h"
#include "common/log.h"

extern server_msg_handle* new_server_msg_handle(
        msg_handle* msg_handle, 
        int port, 
        int64_t log_id) {

    assert(log_id, msg_handle != NULL);

    server_msg_handle* handle = (server_msg_handle*)malloc(sizeof(server_msg_handle));
    if (handle == NULL) {
        LOG(LL_FATAL, log_id, "Malloc server_msg_handle failed");
        return NULL;
    }

    bzero(handle, sizeof(server_msg_handle));
    handle->msg_handle = msg_handle;
    handle->port = port;
    handle->log_id = log_id;
    return handle;
}

static int bind_and_listen(server_msg_handle *handle, int64_t log_id) {
    msg_handle* msg_handle = handle->msg_handle;
    int port = handle->port;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        LOG(LL_ERROR, log_id, "initial socket for port %d failed: %d", port, listen_fd);
        return listen_fd;
    }

    //set server addr_param
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);

    //bind sockfd & addr
    int ret = bind(listen_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "bind to port %d failed: %d", port, ret);
        return ret;
    }

    //listen
    ret = listen(listen_fd, 5);
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "listen to port %d failed: %d", port, ret);
        return ret;
    }

    //have connect request use accept
    struct sockaddr their_addr;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    socklen_t len = sizeof(their_addr);
    while(1) {
        int com_fd = accept(listen_fd, &their_addr, &len);
        if (com_fd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            int err_no = errno;
            LOG(LL_ERROR, log_id, "Accept fd < 0, it is: %d, errno %d.", com_fd, err_no);
            break;
        }

        ret = getnameinfo(&their_addr, len,
                          hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                          NI_NUMERICHOST | NI_NUMERICSERV);
        if (ret == 0) {
            LOG(LL_INFO, log_id, "Accepted connection on descriptor %d "
                                 "(host=%s, port=%s).", com_fd, hbuf, sbuf);
        } else {
            LOG(LL_ERROR, log_id, "Accepted connection on descriptor %d,"
                                  "But getnameinfo failed %d", com_fd, ret);
        }

        conn_id_t conn_id = new_conn(msg_handle, hbuf, atoi(sbuf), com_fd, log_id);
        if (conn_id < 0) {
            LOG(LL_ERROR, log_id, "Call new_conn failed, fd %d.", com_fd);
            break;
        }
    }
    return 0;
}

extern int start_server_messenger(server_msg_handle *handle, int64_t log_id) {
    int ret = start_messager(handle->msg_handle, log_id);
    if (ret == 0) {
        LOG(LL_INFO, log_id, "start messenger for server_messenger success.");
    } else {
        LOG(LL_ERROR, log_id, "start messenger for server_messenger failed: %d.", ret);
        return ret;
    }

    ret = bind_and_listen(handle, log_id);
    ret = stop_messager(handle->msg_handle, log_id);

    return ret;
}
extern int stop_server_messenger(server_msg_handle *handle, int64_t log_id) {
    return 0;
}

extern msg_handle* get_msg_handle(server_msg_handle *handle) {
    return handle->msg_handle;
}
