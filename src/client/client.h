#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

#include <pthread.h>

#include "include/types.h"
#include "include/int_types.h"

#include "common/list.h"

#include "message/messenger.h"
#include "message/msg_write_obj.h"

#define CCEPH_CLIENT_STATE_UNKNOWN  0
#define CCEPH_CLIENT_STATE_NORMAL   1
#define CCEPH_CLIENT_STATE_DESTORY  2

typedef struct {
    char* host;
    int   port;
} osd_id;

typedef struct {
    int osd_count;
    osd_id* osds;
} osdmap;

typedef struct {
    msg_write_obj_req *req;
    struct list_head list_node;
} wait_req;

typedef struct {
    osdmap *osdmap;
    msg_handle *msg_handle;

    int state;

    wait_req wait_req_list;
    pthread_mutex_t wait_req_lock;
    pthread_cond_t wait_req_cond;
} client_handle;

extern client_handle *cceph_new_client_handle(osdmap* osdmap);
extern int cceph_initial_client(client_handle *handle);

extern int cceph_client_write_obj(osdmap* osdmap, int64_t log_id,
                     char* oid, int64_t offset, int64_t length, char* data);

extern int cceph_client_read_obj(osdmap* osdmap, int64_t log_id,
                    char* oid, int64_t offset, int64_t length, char* data);

extern int cceph_client_delete_obj(osdmap* osdmap, int64_t log_id,
                      char* oid);

#endif
