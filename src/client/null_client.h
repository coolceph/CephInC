#ifndef CCEPH_NULL_CLIENT_H
#define CCEPH_NULL_CLIENT_H

#include "client/client.h"

int cceph_null_client_rados_create(void** rados) {
	*rados = NULL;
	return 0;
};

int cceph_null_client_write(const char* object_id, const char* buffer, 
  size_t buffer_size, uint_64t offset) {
	return 0;
}

struct client null_client {
	.name 			= "null_client",
	.rados_create  	= &cceph_null_client_write,
	.write 			= &cceph_null_client_write
};

#endif
