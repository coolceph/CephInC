#ifndef CCEPH_TYPES_H
#define CCEPH_TYPES_H

#include <stdint.h>

typedef int32_t cceph_osd_id_t;

typedef struct {
    cceph_osd_id_t id;

    char* host;
    int   port;
} cceph_osd_entity;

typedef struct {
    int osd_count;
    cceph_osd_entity* osds;
} cceph_osdmap;


#endif
