#ifndef CCEPH_TYPES_H
#define CCEPH_TYPES_H

#include <stdint.h>
#include <stdlib.h>

#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

typedef int8_t  cceph_version_t;
typedef int32_t cceph_epoch_t;
typedef int32_t cceph_osd_id_t;
typedef int32_t cceph_pg_id_t;

#endif
