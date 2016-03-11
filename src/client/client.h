#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

#include <pthread.h>

#include "common/list.h"
#include "common/types.h"

#include "message/messenger.h"
#include "message/msg_write_obj.h"

#define CCEPH_CLIENT_STATE_UNKNOWN  0
#define CCEPH_CLIENT_STATE_NORMAL   1
#define CCEPH_CLIENT_STATE_DESTORY  2

typedef struct {
    char* host;
    int   port;
} cceph_osd_id;

typedef struct {
    int osd_count;
    cceph_osd_id* osds;
} cceph_osdmap;

typedef struct {
    cceph_msg_header *req;
    int ack_count;
    int commit_count;
    int req_count;
    struct cceph_list_head list_node;
} cceph_client_wait_req;

typedef struct {
    cceph_osdmap *osdmap;
    cceph_messenger *messenger;

    int32_t client_id;
    cceph_atomic64_t req_id;
    int state;

    cceph_client_wait_req  wait_req_list;
    pthread_mutex_t        wait_req_lock;
    pthread_cond_t         wait_req_cond; //when req finished, this will be singal
} cceph_client;

extern cceph_client *cceph_client_new(cceph_osdmap* osdmap);
extern int cceph_client_init(cceph_client *client);

extern int cceph_client_write_obj(cceph_client* client,
                    char* oid, int64_t offset, int64_t length, char* data);

extern int cceph_client_read_obj(cceph_client* client, int64_t log_id,
                    char* oid, int64_t offset, int64_t length, char* data);

extern int cceph_client_delete_obj(cceph_client* client, int64_t log_id,
                    char* oid);

extern int TEST_cceph_client_add_req_to_wait_list(cceph_client *client,
        cceph_msg_header *req, int req_count, int64_t log_id);
extern int TEST_cceph_client_send_req_to_osd(cceph_messenger* msger,
        cceph_osd_id *osd, cceph_msg_header* req, int64_t log_id);

#endif
