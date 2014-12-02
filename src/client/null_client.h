#ifndef CCEPH_NULL_CLIENT_H
#define CCEPH_NULL_CLIENT_H

#include <stddef.h>
#include <string.h>

#include "client/client.h"

int cceph_null_client_rados_create(struct cceph_cluster_map **cluster_map);
int cceph_null_client_rados_destory(struct cceph_cluster_map **cluster_map);

int cceph_null_client_rados_connect(struct cceph_cluster_map *cluster_map);
int cceph_null_client_rados_shutdown(struct cceph_cluster_map *cluster_map);

int cceph_null_client_pool_list(struct cceph_cluster_map *cluster_map,
    char *buf, cc_u64 len);
int cceph_null_client_pool_create(struct cceph_cluster_map *cluster_map,
    const char *pool_name);
int cceph_null_client_pool_remove(struct cceph_cluster_map *cluster_map,
    const char *pool_name);

int cceph_null_client_ioctx_create(struct cceph_cluster_map *cluster_map,
    const char *pool_name, struct cceph_client_ioctx_t **ioctx);
int cceph_null_client_ioctx_destory(struct cceph_client_ioctx_t **ioctx);

int cceph_null_client_pool_stat(struct cceph_client_ioctx_t *ioctx,
    struct cceph_client_pool_stat *stats);

int cceph_null_client_read(struct cceph_client_ioctx_t *ioctx,
    const char *oid, char *buf, cc_u64 len, cc_u64 off);
int cceph_null_client_write(struct cceph_client_ioctx_t *ioctx,
    const char* oid, const char* buf, cc_u64 len, cc_u64 off);
int cceph_null_client_write_full(struct cceph_client_ioctx_t *ioctx,
    const char* oid, const char* buf, cc_u64 len);
int cceph_null_client_append(struct cceph_client_ioctx_t *ioctx,
    const char* oid, const char* buf, cc_u64 len);
int cceph_null_client_remove(struct cceph_client_ioctx_t *ioctx,
    const char *oid);
int cceph_null_client_truncate(struct cceph_client_ioctx_t *ioctx,
    const char *oid, cc_u64 size);

void cceph_null_client_register();
void cceph_null_client_unregister();

#endif

