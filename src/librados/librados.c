#include "client/client.h"
#include "librados/librados.h"

int rados_create(rados_t *cluster) {
  return 0;
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

