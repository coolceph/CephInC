#ifndef CCEPH_MESSAGE_SERVER_MESSAGER_H
#define CCEPH_MESSAGE_SERVER_MESSAGER_H

#include "message/messenger.h"

typedef struct {
    msg_handle* msg_handle;
    int port;
    int64_t log_id;
} server_msg_handle;

extern server_msg_handle* new_server_msg_handle(msg_handle* msg_handle, int port, int64_t log_id);

extern int start_server_messenger(server_msg_handle *handle, int64_t log_id);
extern int stop_server_messenger(server_msg_handle *handle, int64_t log_id);

extern msg_handle* get_msg_handle(server_msg_handle *handle);

#endif
