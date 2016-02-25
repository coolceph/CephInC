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
    cceph_log_initial_id(log_prefix);

    int64_t log_id = cceph_log_new_id();
    cceph_messenger* messenger = cceph_messenger_new(&cceph_osd_process_message, NULL, log_id);
    server_cceph_messenger *server_cceph_messenger = new_server_cceph_messenger(messenger, port, log_id);

    start_server_messenger(server_cceph_messenger, log_id);

    return 0;
}

