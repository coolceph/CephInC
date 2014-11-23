#ifndef CCEPH_NULL_CLIENT_H
#define CCEPH_NULL_CLIENT_H

#include <stddef.h>

#include "client/client.h"

int cceph_null_client_rados_create(struct cceph_cluster_map **cluster_map) {
    *cluster_map = NULL;
  return 0;
}
int cceph_null_client_rados_destory(struct cceph_cluster_map **cluster_map) {
    *cluster_map = NULL;
  return 0;
}

int cceph_null_client_rados_connect(struct cceph_cluster_map *cluster_map) {
  return 0;
}
int cceph_null_client_rados_shutdown(struct cceph_cluster_map *cluster_map) {
  return 0;
}

int cceph_null_client_pool_list(struct cceph_cluster_map *cluster_map,
    char *buf, size_t len) {
  char *pool_name = "data";
  int size = len > sizeof(pool_name) ? sizeof(pool_name) : len;
  memcpy(buf, pool_name, size);
  return 0;
}
int cceph_null_client_pool_create(struct cceph_cluster_map *cluster_map,
    const char *pool_name) {
  return 0;
}
int cceph_null_client_pool_remove(struct cceph_cluster_map *cluster_map,
    const char *pool_name) {
  return 0;
}

int cceph_null_client_ioctx_create(struct cceph_cluster_map *cluster_map,
    const char *pool_name, struct cceph_client_ioctx_t **ioctx) {
  *ioctx = NULL;
  return 0;
}
int cceph_null_client_ioctx_destory(struct cceph_client_ioctx_t **ioctx) {
    *ioctx = NULL;
  return 0;
}

int cceph_null_client_pool_stat(struct cceph_client_ioctx_t *ioctx,
    struct cceph_client_pool_stat *stats) {
  return 0;
}

int cceph_null_client_read(struct cceph_client_ioctx_t *ioctx,
    const char *oid, char *buf, size_t len, uint64_t off) {
  return 0;
}
int cceph_null_client_write(struct cceph_client_ioctx_t *ioctx,
    const char* oid, const char* buf, size_t len, uint64_t off) {
  return 0;
}
int cceph_null_client_write_full(struct cceph_client_ioctx_t *ioctx,
    const char* oid, const char* buf, size_t len) {
  return 0;
}
int cceph_null_client_append(struct cceph_client_ioctx_t *ioctx,
    const char* oid, const char* buf, size_t len) {
  return 0;
}
int cceph_null_client_remove(struct cceph_client_ioctx_t *ioctx,
    const char *oid) {
  return 0;
}
int cceph_null_client_truncate(struct cceph_client_ioctx_t *ioctx,
    const char *oid, uint64_t size) {
  return 0;
}

struct cceph_client null_client = {
  .name       = "null_client",

  .rados_create     = cceph_null_client_rados_create,
  .raods_destory    = cceph_null_client_rados_destory,

  .rados_connect    = cceph_null_client_rados_connect,
  .raods_shutdown   = cceph_null_client_rados_shutdown,

  .pool_list        = cceph_null_client_pool_list,
  .pool_create      = cceph_null_client_pool_create,
  .pool_remove      = cceph_null_client_pool_remove,

  .ioctx_create     = cceph_null_client_ioctx_create,
  .ioctx_destory    = cceph_null_client_ioctx_destory,

  .pool_stat        = cceph_null_client_pool_stat,

  .read             = cceph_null_client_read,
  .write            = cceph_null_client_write,
  .write_full       = cceph_null_client_write_full,
  .append           = cceph_null_client_append,
  .remove           = cceph_null_client_remove,
  .truncate         = cceph_null_client_truncate,
};

void cceph_null_client_register() {
  cceph_client_register(CCEPH_CLIENT_TYPE_NULL, &null_client);
}
void cceph_null_client_unregister() {
  cceph_client_unregister(CCEPH_CLIENT_TYPE_NULL);
}

#endif
