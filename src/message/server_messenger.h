#ifndef CCEPH_MESSAGE_SERVER_MESSAGER_H
#define CCEPH_MESSAGE_SERVER_MESSAGER_H

#include "message/messenger.h"

typedef struct {
    cceph_messenger* messenger;
    int port;
    int64_t log_id;
} cceph_server_messenger;

extern cceph_server_messenger* new_cceph_server_messenger(
        cceph_messenger* messenger, int port, int64_t log_id);

extern int cceph_server_messenger_start(
        cceph_server_messenger *server_messenger, int64_t log_id);
extern int cceph_server_messenger_stop(
        cceph_server_messenger *server_messenger, int64_t log_id);

extern cceph_messenger* cceph_server_messenger_get_messenger(
        cceph_server_messenger *server_messenger);

#endif
