#ifndef CCEPH_PG_H
#define CCEPH_PG_H

#include "common/rbtree.h"
#include "osd/types.h"

typedef struct {
    cceph_pg_id_t pg_id;
    int8_t        state;

    pthread_mutex_t lock;

    cceph_rb_node node;
} cceph_pg;

extern int cceph_pg_new(
        cceph_pg**    pg,
        cceph_pg_id_t pg_id,
        int64_t       log_id);

extern int cceph_pg_free(
        cceph_pg** node,
        int64_t    log_id);

CCEPH_DEFINE_MAP(pg, cceph_pg_id_t, pg_id);

#endif
