#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

#include "include/types.h"
#include "include/int_types.h"

#include "message/messenger.h"

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
} client_handle;

extern client_handle *new_client_handle(osdmap* osdmap);

extern int client_write_obj(osdmap* osdmap, int64_t log_id,
                     char* oid, int64_t offset, int64_t length, char* data);

extern int client_read_obj(osdmap* osdmap, int64_t log_id,
                    char* oid, int64_t offset, int64_t length, char* data);

extern int client_delete_obj(osdmap* osdmap, int64_t log_id,
                      char* oid);

#endif
