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
#include "common/errno.h"
#include "common/log.h"

extern cceph_server_messenger* new_cceph_server_messenger(
        cceph_messenger* messenger,
        int port,
        int64_t log_id) {

    assert(log_id, messenger != NULL);
    assert(log_id, messenger->state == CCEPH_MESSENGER_STATE_UNKNOWN);

    cceph_server_messenger* server_messenger = (cceph_server_messenger*)malloc(sizeof(cceph_server_messenger));
    if (server_messenger == NULL) {
        LOG(LL_FATAL, log_id, "Malloc cceph_server_messenger failed.");
        return NULL;
    }

    bzero(server_messenger, sizeof(cceph_server_messenger));
    server_messenger->messenger = messenger;
    server_messenger->port = port;
    server_messenger->log_id = log_id;
    return server_messenger;
}
extern int free_cceph_server_messenger(
        cceph_server_messenger** server_messenger, int64_t log_id) {
    assert(log_id, *server_messenger != NULL);

    free(*server_messenger);
    *server_messenger = NULL;

    return 0;
}

static int bind_and_listen(cceph_server_messenger *server_messenger, int64_t log_id) {
    cceph_messenger* messenger = server_messenger->messenger;
    int port = server_messenger->port;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        LOG(LL_ERROR, log_id, "Initial socket for port %d failed, errno %d.", port, listen_fd);
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
        LOG(LL_ERROR, log_id, "Bind to port %d failed, errno %d.", port, ret);
        return ret;
    }

    //listen
    ret = listen(listen_fd, 5); //TODO: Maybe an opinion
    if (ret == -1) {
        LOG(LL_ERROR, log_id, "Listen to port %d failed, errno %d.", port, ret);
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
                                  "But getnameinfo failed %d.", com_fd, ret);
        }

        cceph_conn_id_t conn_id = cceph_messenger_add_conn(messenger, hbuf, atoi(sbuf), com_fd, log_id);
        if (conn_id < 0) {
            LOG(LL_ERROR, log_id, "Call cceph_messenger_add_conn failed, fd %d, errno %d(%s).",
                    com_fd, conn_id, errno_str(conn_id));
            break;
        }
    }
    return 0;
}

extern int cceph_server_messenger_start(
        cceph_server_messenger *server_messenger, int64_t log_id) {
    int ret = cceph_messenger_start(server_messenger->messenger, log_id);
    if (ret == 0) {
        LOG(LL_INFO, log_id, "start messenger for server_messenger success.");
    } else {
        LOG(LL_ERROR, log_id, "start messenger for server_messenger failed. errno %d(%s).",
                ret, errno_str(ret));
        return ret;
    }

    //The bind_and_listen will block the thread
    ret = bind_and_listen(server_messenger, log_id);

    //Here means the listen operation is aborted, we should stop the messenger
    ret = cceph_messenger_stop(server_messenger->messenger, log_id);

    return ret;
}
extern int cceph_server_messenger_stop(
        cceph_server_messenger *server_messenger, int64_t log_id) {
    //TODO: Do we actually need it?
    assert(log_id, server_messenger != NULL);
    return 0;
}

extern cceph_messenger* cceph_server_messenger_get_messenger(
        cceph_server_messenger *server_messenger) {
    return server_messenger->messenger;
}
