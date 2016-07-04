#ifndef CCEPH_OSDMAP_H
#define CCEPH_OSDMAP_H

#include "common/buffer.h"
#include "common/encode.h"

typedef struct {
    cceph_osd_id_t id;

    char* host; //This will not be persisted
    int   port; //This will not be persisted
} cceph_osd_entity;

typedef struct {
    cceph_epoch_t epoch;

    int pg_count;

    int osd_count;
    cceph_osd_entity* osds;

} cceph_osdmap;

CCEPH_DEFINE_ENCODE_METHOD(osd_entity);
CCEPH_DEFINE_ENCODE_METHOD(osdmap);

#endif
