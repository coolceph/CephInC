#ifndef CCEPH_PG_MAP_H
#define CCEPH_PG_MAP_H

#include "osd/types.h"

typedef struct {
    cceph_pg* pg;

    cceph_rb_node node;
} cceph_pg_map_node;

extern int cceph_pg_map_node_new(
        cceph_pg*           pg,
        cceph_pg_map_node** node,
        int64_t             log_id);

extern int cceph_pg_map_node_free(
        cceph_pg_map_node** node,
        int64_t             log_id);

extern int cceph_pg_map_node_free_tree(
        cceph_rb_root*      tree,
        int64_t             log_id);

extern int cceph_pg_map_node_insert(
        cceph_rb_root       *root,
        cceph_pg_map_node   *node,
        int64_t             log_id);

extern int cceph_pg_map_node_remove(
        cceph_rb_root       *root,
        cceph_pg_map_node   *node,
        int64_t             log_id);

extern int cceph_pg_map_node_search(
        cceph_rb_root*      root,
        cceph_pg_id_t       pg_id,
        cceph_pg_map_node** result,
        int64_t             log_id);

#endif
