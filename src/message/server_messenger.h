#ifndef CCEPH_MESSAGE_SERVER_MESSAGER_H
#define CCEPH_MESSAGE_SERVER_MESSAGER_H

#include "message/messenger.h"

typedef struct {
    cceph_messenger* cceph_messenger;
    int port;
    int64_t log_id;
} server_cceph_messenger;

extern server_cceph_messenger* new_server_cceph_messenger(cceph_messenger* cceph_messenger, int port, int64_t log_id);

extern int start_server_messenger(server_cceph_messenger *handle, int64_t log_id);
extern int stop_server_messenger(server_cceph_messenger *handle, int64_t log_id);

extern cceph_messenger* get_cceph_messenger(server_cceph_messenger *handle);

#endif
