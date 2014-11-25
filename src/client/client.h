#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

#include "include/types.h"
#include "include/int_types.h"

#define MAX_CLIENT_TYPES 16

enum CCEPH_CLIENT_TYPE {
  CCEPH_CLIENT_TYPE_NULL = 0,
  CCEPH_CLIENT_TYPE_MEM = 1,
  CCEPH_CLIENT_TYPE_NORMAL = 2,
  CCEPH_CLIENT_TYPE_MAX = 3,
};

struct cceph_client_ioctx_t {

};

struct cceph_client_pool_stat {
  cc_u64 num_bytes;
  cc_u64 num_objects;
};

struct cceph_client {
  char* name;

  int (*rados_create)(struct cceph_cluster_map **cluster_map);
  int (* raods_destory)(struct cceph_cluster_map **cluster_map);

  int (* rados_connect)(struct cceph_cluster_map *cluster_map);
  int (* raods_shutdown)(struct cceph_cluster_map *cluster_map);

  int (* pool_list)(struct cceph_cluster_map *cluster_map,
    char *buf, cc_size_t len);
  int (* pool_create)(struct cceph_cluster_map *cluster_map,
    const char *pool_name);
  int (* pool_remove)(struct cceph_cluster_map *cluster_map,
    const char *pool_name);

  int (* ioctx_create)(struct cceph_cluster_map *cluster_map,
    const char *pool_name, struct cceph_client_ioctx_t **ioctx);
  int (* ioctx_destory)(struct cceph_client_ioctx_t **ioctx);

  int (* pool_stat)(struct cceph_client_ioctx_t *ioctx,
    struct cceph_client_pool_stat *stats);

  int (* read)(struct cceph_client_ioctx_t *ioctx,
    const char *oid, char *buf, cc_size_t len, cc_u64 off);
  int (* write)(struct cceph_client_ioctx_t *ioctx,
    const char*, const char*, cc_size_t len, cc_u64 off);
  int (* write_full)(struct cceph_client_ioctx_t *ioctx,
    const char*, const char*, cc_size_t len);
  int (* append)(struct cceph_client_ioctx_t *ioctx,
    const char*, const char*, cc_size_t len);
  int (* remove)(struct cceph_client_ioctx_t *ioctx,
    const char *oid);
  int (* truncate)(struct cceph_client_ioctx_t *ioctx,
    const char *oid, cc_u64 size);
};

void cceph_client_register(enum CCEPH_CLIENT_TYPE type, struct cceph_client *client);
void cceph_client_unregister(enum CCEPH_CLIENT_TYPE type);

#endif
