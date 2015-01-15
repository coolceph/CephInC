#ifndef CCEPH_CLUSTER_MAP_H
#define CCEPH_CLUSTER_MAP_H

#include "include/types.h"

/*
  Read cluster map from describe map.
  The default path of the file is /etc/cceph/cluster.describe
*/
int cceph_cluster_map_read_from_describe_file(struct cceph_cluster_map *map);

#endif