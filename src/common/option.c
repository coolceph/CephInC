#include "option.h"

#include <stdlib.h>
#include <strings.h>

#include "common/assert.h"
#include "common/types.h"

cceph_option g_cceph_option;

int cceph_option_initial() {
    bzero(&g_cceph_option, sizeof(cceph_option));
    g_cceph_option.client_msg_workthread_count = 2;
    g_cceph_option.client_debug_check_duplicate_req_when_ack = 1;

    g_cceph_option.osd_msg_workthread_count = 2;
    g_cceph_option.osd_reply_write_commit_to_client = 1;
    g_cceph_option.osd_port_base = 10000;
    return 0;
}
int cceph_option_load(const char* option_path, int64_t log_id) {
    assert(log_id, option_path != NULL);
    return 0;
}
