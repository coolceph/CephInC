#include "common/log.h"
#include "message/server_messenger.h"
#include "osd/osd.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    int32_t log_prefix = 201;
    cceph_initial_log_id(log_prefix);

    int64_t log_id = cceph_new_log_id();
    msg_handle* msg_handle = new_msg_handle(&osd_process_message, NULL, log_id);
    server_msg_handle *server_msg_handle = new_server_msg_handle(msg_handle, port, log_id);

    start_server_messenger(server_msg_handle, log_id);

    return 0;
}

