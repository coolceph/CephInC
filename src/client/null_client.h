#ifndef CCEPH_NULL_CLIENT_H
#define CCEPH_NULL_CLIENT_H

#include "client/client.h"

extern int cceph_null_client_write(const char* object_id, const char* buffer, 
  size_t buffer_size, uint_64t offset);

#endif
