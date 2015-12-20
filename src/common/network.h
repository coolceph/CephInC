#ifndef CCEPH_NETWORK_H
#define CCEPH_NETWORK_H

#include "include/types.h"
#include "common/log.h"

extern int send_int8(int fd, int8_t value);

extern int send_int64(int fd, int64_t value);

extern int send_string(int fd, char* string);

extern int send_data(int fd, int64_t length, char* data);


extern int read_int8(int data_fd, int8_t* value);

extern int read_int64(int data_fd, int64_t* value);

extern int read_string(int data_fd, int16_t *size, char **string);

extern int read_data(int data_fd, int64_t *size, char **data);

#endif
