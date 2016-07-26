#ifndef CCEPH_PG_MAP_H
#define CCEPH_PG_MAP_H

#include "osd/types.h"

extern int cceph_pg_map_insert(
        cceph_rb_root *root,
        cceph_pg      *node,
        int64_t       log_id);

extern int cceph_pg_map_remove(
        cceph_rb_root *root,
        cceph_pg      *node,
        int64_t       log_id);

extern int cceph_pg_map_search(
        cceph_rb_root *root,
        cceph_pg_id_t pg_id,
        cceph_pg      **result,
        int64_t       log_id);

#endif
