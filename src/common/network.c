#include "common/network.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int send_int8(int fd, int8_t value) {
    int ret = send(fd, &value, sizeof(int8_t), 0);
    if (ret != sizeof(int8_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send int8_t %d error: %d", value, errno);
        return errno;
    }
    return 0;
}
int send_int64(int fd, int64_t value) {
    int ret = send(fd, &value, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send int64_t %ld error: %d", value, errno);
        return errno;
    }
    return 0;
}
int send_string(int fd, char* string) {
    int16_t size = strlen(string);
    
    int ret = send(fd, &size, sizeof(int16_t), 0);
    if (ret != sizeof(int16_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send string %s error: %d", string, errno);
        return errno;
    }
    ret = send(fd, string, size, 0);
    if (ret != size) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send string %s error: %d", string, errno);
        return errno;
    }
    return 0;
}
int send_data(int fd, int64_t length, char* data) {
    int ret = send(fd, &length, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send data error: %d, length: %ld", errno, length);
        return errno;
    }
    ret = send(fd, data, length, 0);
    if (ret != length) {
        int errno = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send data error: %d, length: %ld", errno, length);
        return errno;
    }

    return 0;
}
