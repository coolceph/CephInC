#ifndef CCEPH_MESSAGE_IO_H
#define CCEPH_MESSAGE_IO_H

#include "include/types.h"
#include "common/log.h"

extern int send_int8(int fd, int8_t value, int64_t log_id);
extern int send_int64(int fd, int64_t value, int64_t log_id);
extern int send_string(int fd, char* string, int64_t log_id);
extern int send_data(int fd, int64_t length, char* data, int64_t log_id);

extern int recv_int8(int data_fd, int8_t* value, int64_t log_id);
extern int recv_int64(int data_fd, int64_t* value, int64_t log_id);
extern int recv_string(int data_fd, int16_t *size, char **string, int64_t log_id);
extern int recv_data(int data_fd, int64_t *size, char **data, int64_t log_id);

extern int TEST_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id);
#endif
