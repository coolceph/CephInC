#ifndef CCEPH_OPTION_H
#define CCEPH_OPTION_H

#include "common/assert.h"
#include "common/types.h"

typedef struct {
    int client_msg_workthread_count;
    int client_debug_check_duplicate_req_when_ack;

    int osd_msg_workthread_count;
    int osd_reply_write_commit_to_client;
} cceph_option;

extern cceph_option g_cceph_option;

extern int cceph_option_init();

extern int cceph_option_load(const char* option_path, int64_t log_id);

#endif
