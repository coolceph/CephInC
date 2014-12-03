#include "librados/librados.h"

#include "common/config.h"
#include "common/global.h"

#include "client/client.h"
#include "client/null_client.h"

struct cceph_client *client = NULL;

void init_clients() {
  cceph_null_client_register();
  client = cceph_get_client(g_conf->client_type);
}

int rados_create(rados_t *cluster) {

  cceph_init_global();
  
  init_client();

  struct cceph_cluster_map *cluster_map = cceph_cluster_map_read_from_desc();
  *cluster = cluster_map;

  return client->rados_create(cluster_map);
}

void rados_destory(rados_t cluster) {
}

int rados_connect(rados_t cluster) {
  return 0;
}

void rados_shutdown(rados_t cluster) {
}

int rados_pool_list(rados_t cluster, char *buf, cc_size_t len) {
  return 0;
}

int rados_pool_create(rados_t cluster, const char *pool_name) {
  return 0;
}

int rados_pool_remove(rados_t cluster, const char *pool_name) {
  return 0;
}

int rados_ioctx_create(rados_t cluster, const char *pool_name, rados_ioctx_t *ioctx) {
  return 0;
}

void rados_ioctx_destroy(rados_ioctx_t io) {
}

int rados_ioctx_pool_stat(rados_ioctx_t io, struct rados_pool_stat_t *stats) {
  return 0;
}

int rados_write(rados_ioctx_t io, const char *oid, const char *buf, cc_size_t len,   cc_u64 off) {
  return 0;
}

int rados_write_full(rados_ioctx_t io, const char *oid, const char *buf, cc_size_t len) {
  return 0;
}

int rados_append(rados_ioctx_t io, const char *oid, const char *buf, cc_size_t len) {
  return 0;
}

int rados_read(rados_ioctx_t io, const char *oid, char *buf, cc_size_t len,   cc_u64 off) {
  return 0;
}

int rados_remove(rados_ioctx_t io, const char *oid) {
  return 0;
}

int rados_truncate(rados_ioctx_t io, const char *oid,   cc_u64 size) {
  return 0;
}

