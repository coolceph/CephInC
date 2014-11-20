#ifndef CCEPH_TYPES_H
#define CCEPH_TYPES_H

#include <netinet/in.h>

enum cceph_entity_type {
	CCEPH_CLIENT_ENTITY_TYPE_MON,
	CCEPH_CLIENT_ENTITY_TYPE_OSD,
	CCEPH_CLIENT_ENTITY_TYPE_CLIENT
};

struct cceph_entity {
	enum cceph_client_entity_type type;
	int id;
	struct sockaddr_in addr; 
};

struct cceph_osd_map {
	int osd_count;
	struct cceph_entity *osds;
};

struct cceph_cluster_map {
	struct cceph_client_entity mon;
	struct cceph_osd_map osd_map;
};

#endif