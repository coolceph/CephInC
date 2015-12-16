#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

#include "include/types.h"
#include "include/int_types.h"

struct osd {
    char* host;
    int   port;
};

struct osdmap {
    int osd_count;
    struct osd* osds;
};

extern int write_obj(struct osdmap* osdmap,
                     char* oid, int64_t offset, int64_t length, char* data);

extern int read_obj(struct osdmap* osdmap,
                    char* oid, int64_t offset, int64_t length, char* data);

extern int delete_obj(struct osdmap* osdmap,
                      char* oid);

#endif
