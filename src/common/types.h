#ifndef CCEPH_TYPES_H
#define CCEPH_TYPES_H

#include <stdint.h>
#include <stdlib.h>

typedef int32_t cceph_epoch_t;
typedef int32_t cceph_osd_id_t;
typedef int32_t cceph_pg_id_t;

typedef struct {
    cceph_osd_id_t id;

    char* host;
    int   port;
} cceph_osd_entity;

typedef struct {
    cceph_epoch_t epoch;

    int32_t pg_count;

    int osd_count;
    cceph_osd_entity* osds;

} cceph_osdmap;


#endif
