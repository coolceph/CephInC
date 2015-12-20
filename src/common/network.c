#include "common/network.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static int read_from_conn(int data_fd, void* buf, size_t size);

int send_int8(int fd, int8_t value) {
    int ret = send(fd, &value, sizeof(int8_t), 0);
    if (ret != sizeof(int8_t)) {
        int err_no = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send int8_t %d error: %d", value, err_no);
        return err_no;
    }
    return 0;
}
int send_int64(int fd, int64_t value) {
    int ret = send(fd, &value, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int err_no = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send int64_t %ld error: %d", value, err_no);
        return err_no;
    }
    return 0;
}
int send_string(int fd, char* string) {
    int16_t size = strlen(string);
    
    int ret = send(fd, &size, sizeof(int16_t), 0);
    if (ret != sizeof(int16_t)) {
        int err_no = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send string %s error: %d", string, err_no);
        return err_no;
    }
    ret = send(fd, string, size, 0);
    if (ret != size) {
        int err_no = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send string %s error: %d", string, err_no);
        return err_no;
    }
    return 0;
}
int send_data(int fd, int64_t length, char* data) {
    int ret = send(fd, &length, sizeof(int64_t), 0);
    if (ret != sizeof(int64_t)) {
        int err_no = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send data error: %d, length: %ld", err_no, length);
        return err_no;
    }
    ret = send(fd, data, length, 0);
    if (ret != length) {
        int err_no = ret < 0 ? ret : -1;
        LOG(LL_ERROR, "send data error: %d, length: %ld", err_no, length);
        return err_no;
    }

    return 0;
}

int read_int8(int data_fd, int8_t* value) {
    return read_from_conn(data_fd, value, sizeof(*value)) == sizeof(*value) ? 0 : -1;
}
int read_int64(int data_fd, int64_t* value) {
    return read_from_conn(data_fd, value, sizeof(*value)) == sizeof(*value) ? 0 : -1;
}
int read_string(int data_fd, int16_t *size, char **string) {
    if(read_from_conn(data_fd, size, sizeof(*size)) != sizeof(*size)) {
        return -1;
    }
    
    *string = malloc(*size + 1);
    (*string)[*size] = '\0';
    if (read_from_conn(data_fd, *string, *size) != *size) {
        free(*string);
        *string = NULL;
        return -1;
    }
    return 0;
}
int read_data(int data_fd, int64_t *size, char **data) {
    if(read_from_conn(data_fd, size, sizeof(*size)) != sizeof(*size)) {
        return -1;
    }
    
    *data = malloc(*size);
    if (read_from_conn(data_fd, *data, *size) != *size) {
        free(*data);
        *data = NULL;
        return -1;
    }
    return 0;
}

static int read_from_conn(int data_fd, void* buf, size_t size) {
    int total = 0;
    int closed = 0;
    while(size > 0) {
        int count = recv(data_fd, buf, size, 0);
        if (count == -1 && errno != EAGAIN) {
            /* If errno == EAGAIN, that means we have read all
               data. So go back to the main loop. */
            LOG(LL_ERROR, "read close");
            closed = 1;
            break;
        }

        if (count == -1 && errno == EAGAIN && size > 0) {
            LOG(LL_ERROR, "incomplete read, wait and retry to read");
            sleep(1);
            continue;
        }

        if (count == 0) {
            if (size > 0) LOG(LL_ERROR, "incomplete read");
            LOG(LL_ERROR, "read close by ret 0");
            closed = 1;
            break;
        }
        
        buf  += count;
        size -= count;
        total += count;
    }
    if (closed) {
        LOG(LL_INFO, "Closed connection on descriptor %d", data_fd);
        close(data_fd);
    }
    return total;
}

