#ifndef CCEPH_MESSAGE_IO_H
#define CCEPH_MESSAGE_IO_H

#include "include/types.h"
#include "common/log.h"

#define CCEPH_SEND_FIELD(name, type, value)                                 \
ret = send_##type(fd, value, log_id);                                       \
if (ret < 0) {                                                              \
    LOG(LL_ERROR, log_id, "Send #name error, fd %d, errno %d.", fd, ret);   \
    return ret;                                                             \
}
#define CCEPH_SEND_DATA_FIELD(name, length, value)                          \
ret = send_data(fd, length, value, log_id);                                 \
if (ret < 0) {                                                              \
    LOG(LL_ERROR, log_id, "Send #name error, fd %d, errno %d.", fd, ret);   \
    return ret;                                                             \
}

#define CCEPH_RECV_FIELD(name, type, value)                                 \
ret = recv_##type(fd, value, log_id);                                       \
if (ret < 0) {                                                              \
    LOG(LL_ERROR, log_id, "Recv "#name" error, fd %d, errno %d.", fd, ret);   \
    return ret;                                                             \
}
#define CCEPH_RECV_STRING_FIELD(name, length, value)                        \
ret = recv_string(fd, length, value, log_id);                               \
if (ret < 0) {                                                              \
    LOG(LL_ERROR, log_id, "Recv "#name" error, fd %d, errno %d.", fd, ret);   \
    return ret;                                                             \
}
#define CCEPH_RECV_DATA_FIELD(name, length, value)                          \
ret = recv_data(fd, length, value, log_id);                                 \
if (ret < 0) {                                                              \
    LOG(LL_ERROR, log_id, "Recv "#name" error, fd %d, errno %d.", fd, ret);   \
    return ret;                                                             \
}


extern int send_int8(int fd, int8_t value, int64_t log_id);
extern int send_int16(int fd, int16_t value, int64_t log_id);
extern int send_int32(int fd, int32_t value, int64_t log_id);
extern int send_int64(int fd, int64_t value, int64_t log_id);
extern int send_string(int fd, char* string, int64_t log_id);
extern int send_data(int fd, int64_t length, char* data, int64_t log_id);

extern int recv_int8(int data_fd, int8_t* value, int64_t log_id);
extern int recv_int16(int data_fd, int16_t* value, int64_t log_id);
extern int recv_int32(int data_fd, int32_t* value, int64_t log_id);
extern int recv_int64(int data_fd, int64_t* value, int64_t log_id);
extern int recv_string(int data_fd, int16_t *size, char **string, int64_t log_id);
extern int recv_data(int data_fd, int64_t *size, char **data, int64_t log_id);

extern int TEST_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id);
#endif
