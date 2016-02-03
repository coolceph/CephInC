#include "message/server_messenger.h"

#include <stdlib.h>
#include <strings.h>

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

extern int start_server_messenger(server_msg_handle *handle, int64_t log_id) {
    return 0;
}
extern int stop_server_messenger(server_msg_handle *handle, int64_t log_id) {
    return 0;
}

extern msg_handle* get_msg_handle(server_msg_handle *handle) {
    return handle->msg_handle;
}
