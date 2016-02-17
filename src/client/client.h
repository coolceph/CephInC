#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

#include "include/types.h"
#include "include/int_types.h"

#include "message/messenger.h"

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
    osdmap *osdmap;
    msg_handle *msg_handle;

    int state;
} client_handle;

extern client_handle *cceph_new_client_handle(osdmap* osdmap);
extern int cceph_init_client(client_handle *handle);

extern int cceph_client_write_obj(osdmap* osdmap, int64_t log_id,
                     char* oid, int64_t offset, int64_t length, char* data);

extern int cceph_client_read_obj(osdmap* osdmap, int64_t log_id,
                    char* oid, int64_t offset, int64_t length, char* data);

extern int cceph_client_delete_obj(osdmap* osdmap, int64_t log_id,
                      char* oid);

#endif
