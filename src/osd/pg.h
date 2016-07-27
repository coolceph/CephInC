#ifndef CCEPH_PG_H
#define CCEPH_PG_H

#include "osd/types.h"

extern int cceph_pg_new(
        cceph_pg**    pg,
        cceph_pg_id_t pg_id,
        int64_t       log_id);

extern int cceph_pg_free(
        cceph_pg** node,
        int64_t    log_id);

#endif
