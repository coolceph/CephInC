#ifndef CCEPH_OSDMAP_H
#define CCEPH_OSDMAP_H

typedef struct {
    cceph_osd_id_t id;

    char* host; //This will not be persisted
    int   port; //This will not be persisted
} cceph_osd_entity;

typedef struct {
    cceph_epoch_t epoch;

    int32_t pg_count;

    int osd_count;
    cceph_osd_entity* osds;

} cceph_osdmap;

#endif
