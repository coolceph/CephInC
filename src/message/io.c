#include "message/io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#include "common/errno.h"

static int cceph_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id);

int cceph_send_int8(int fd, int8_t value, int64_t log_id) {
    int ret = send(fd, &value, sizeof(int8_t), 0);
    if (ret != sizeof(int8_t)) {
        int err_no = ret < 0 ? ret : errno;
        err_no = err_no >= 0 ? -1 : err_no;
        LOG(LL_ERROR, log_id, "send int8_t %d error: %d", value, err_no);
        return err_no;
    }
    return 0;
}
int cceph_send_int16(int fd, int16_t value, int64_t log_id) {
    int ret = send(fd, &value, sizeof(int16_t), 0);
    if (ret != sizeof(int16_t)) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send int16_t %d error: %d", value, err_no);
        return err_no;
    }
    return 0;
}
int cceph_send_int32(int fd, int32_t value, int64_t log_id) {
    int ret = send(fd, &value, sizeof(int32_t), 0);
    if (ret != sizeof(int32_t)) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send int32_t %d error: %d", value, err_no);
        return err_no;
    }
    return 0;
}
int cceph_send_int64(int fd, int64_t value, int64_t log_id) {
    int ret = send(fd, &value, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send int64_t %ld error: %d", value, err_no);
        return err_no;
    }
    return 0;
}
int cceph_send_string(int fd, char* string, int64_t log_id) {
    int16_t size = strlen(string);
    
    int ret = send(fd, &size, sizeof(int16_t), 0);
    if (ret != sizeof(int16_t)) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send string %s error: %d", string, err_no);
        return err_no < 0 ? err_no : -1;
    }
    ret = send(fd, string, size, 0);
    if (ret != size) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send string %s error: %d", string, err_no);
        return err_no < 0 ? err_no : -1;
    }
    return 0;
}
int cceph_send_data(int fd, int64_t length, char* data, int64_t log_id) {
    int ret = send(fd, &length, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send data error: %d, length: %ld", err_no, length);
        return err_no < 0 ? err_no : -1;
    }
    ret = send(fd, data, length, 0);
    if (ret != length) {
        int err_no = ret < 0 ? ret : errno;
        LOG(LL_ERROR, log_id, "send data error: %d, length: %ld", err_no, length);
        return err_no < 0 ? err_no : -1;
    }

    return 0;
}

int cceph_recv_int8(int data_fd, int8_t* value, int64_t log_id) {
    int ret = cceph_recv_from_conn(data_fd, value, sizeof(int8_t), log_id);
    return (ret == sizeof(int8_t)) ? 0 : (ret < 0 ? ret : -1);
}
int cceph_recv_int16(int data_fd, int16_t* value, int64_t log_id) {
    int ret = cceph_recv_from_conn(data_fd, value, sizeof(int16_t), log_id);
    return (ret == sizeof(int16_t)) ? 0 : (ret < 0 ? ret : -1);
}
int cceph_recv_int32(int data_fd, int32_t* value, int64_t log_id) {
    int ret = cceph_recv_from_conn(data_fd, value, sizeof(int32_t), log_id);
    return (ret == sizeof(int32_t)) ? 0 : (ret < 0 ? ret : -1);
}
int cceph_recv_int64(int data_fd, int64_t* value, int64_t log_id) {
    int ret = cceph_recv_from_conn(data_fd, value, sizeof(int64_t), log_id);
    return (ret == sizeof(int64_t)) ? 0 : (ret < 0 ? ret : -1);
}
int cceph_recv_string(int data_fd, int16_t *size, char **string, int64_t log_id) {
    int ret = cceph_recv_from_conn(data_fd, size, sizeof(int16_t), log_id);
    if (ret != sizeof(int16_t)) return (ret < 0 ? ret : -1);
    
    *string = malloc(*size + 1);
    (*string)[*size] = '\0';
    ret = cceph_recv_from_conn(data_fd, *string, *size, log_id);
    if (ret < 0 || ret != *size) {
        free(*string);
        *string = NULL;
    }
    return (ret == *size) ? 0 : (ret < 0 ? ret : -1);
}
int cceph_recv_data(int data_fd, int64_t *size, char **data, int64_t log_id) {
    int ret = cceph_recv_from_conn(data_fd, size, sizeof(*size), log_id);
    if (ret != sizeof(int64_t)) return (ret < 0 ? ret : -1);

    *data = malloc(*size);
    ret = cceph_recv_from_conn(data_fd, *data, *size, log_id);
    if (ret < 0 || ret != *size) {
        free(*data);
        *data = NULL;
    }
    return (ret == *size) ? 0 : (ret < 0 ? ret : -1);
}

static int cceph_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id) {
    int total = 0;
    while(size > 0) {
        int count = recv(data_fd, buf, size, 0);
        if (count == -1 && errno != EAGAIN) {
            LOG(LL_ERROR, log_id, "conn closed when read, fd %d, errno %d.", data_fd, errno);
            return CCEPH_ERR_CONN_CLOSED; //messenger will close it
        }

        if (count == -1 && errno == EAGAIN && size > 0) {
            LOG(LL_INFO, log_id, "incomplete read, retry");
            continue;
        }

        if (count == 0) {
            LOG(LL_ERROR, log_id, "conn closed when read, ret 0, fd %d, already %d", data_fd, total);
            return CCEPH_ERR_CONN_CLOSED; //messenger will close it
        }
        
        buf   += count;
        size  -= count;
        total += count;
    }
    return total;
}

extern int TEST_cceph_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id) {
    return cceph_recv_from_conn(data_fd, buf, size, log_id);
}
