extern "C" {
#include "common/errno.h"
#include "message/io.h"
}

#include <errno.h>

#include "bhook.h"
#include "gtest/gtest.h"

char* fname_recv = (char*)"recv";
char* fname_send = (char*)"send";
char* fname_cceph_recv_from_conn = (char*)"cceph_recv_from_conn";

ssize_t MOCK_cceph_recv_from_conn_recv_normal(int sockfd, void *buf, size_t len, int flags) {
    EXPECT_EQ(1, sockfd);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ((size_t)16, len);
    EXPECT_EQ(0, flags);

    return len;
}
ssize_t MOCK_cceph_recv_from_conn_recv_closed(int sockfd, void *buf, size_t len, int flags) {
    EXPECT_EQ(1, sockfd);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ((size_t)16, len);
    EXPECT_EQ(0, flags);

    errno = EAGAIN + 1;
    return -1;
}
ssize_t MOCK_cceph_recv_from_conn_recv_again(int sockfd, void *buf, size_t len, int flags) {
    static int call_time = -1;
    call_time++;

    EXPECT_EQ(1, sockfd);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ(0, flags);

    if (call_time == 0) {
        EXPECT_EQ(16, (int)len);
        return 4;
    } else if (call_time == 1) {
        EXPECT_EQ(16 - 4, (int)len);
        errno = EAGAIN;
        return -1;
    } else if (call_time == 2) {
        EXPECT_EQ(16 - 4, (int)len);
        return 12;
    }

    EXPECT_TRUE(0 == 1);
    return -1;
}
//static int cceph_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id) {
TEST(message_io, cceph_recv_from_conn) {
    int data_fd = 1;
    char buf[16];
    size_t size = 16;
    int64_t log_id = 1;

    //Case: conn normal
    attach_and_enable_func(fname_recv, (void*)&MOCK_cceph_recv_from_conn_recv_normal);
    EXPECT_EQ(size, (size_t)TEST_cceph_recv_from_conn(data_fd, buf, size, log_id));
    detach_func(fname_recv);

    //Case: conn closed
    attach_and_enable_func(fname_recv, (void*)&MOCK_cceph_recv_from_conn_recv_closed);
    EXPECT_EQ(CCEPH_ERR_CONN_CLOSED, (int)TEST_cceph_recv_from_conn(data_fd, buf, size, log_id));
    detach_func(fname_recv);

    //Case: conn again
    attach_and_enable_func(fname_recv, (void*)&MOCK_cceph_recv_from_conn_recv_again);
    EXPECT_EQ(size, (size_t)TEST_cceph_recv_from_conn(data_fd, buf, size, log_id));
    detach_func(fname_recv);
}

int MOCK_recv_int8_cceph_recv_from_conn(int data_fd, void* buf, size_t size, int64_t log_id) {
    EXPECT_EQ(sizeof(int8_t), size);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ(122, log_id);

    int8_t value = 37;
    memcpy(buf, &value, size);

    if (data_fd == 1) return 1; 
    if (data_fd == 2) return 0; 
    if (data_fd == 3) return -2; 

    EXPECT_TRUE(0 == 1);
    return 0;
}
TEST(message_io, recv_int8) {
    attach_and_enable_func_lib(fname_cceph_recv_from_conn, (void*)&MOCK_recv_int8_cceph_recv_from_conn);

    //Case: normal
    int8_t value   = 0;
    int ret = cceph_recv_int8(1, &value, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(37, value);

    //Case: imcomplete read
    value  = 0;
    ret = cceph_recv_int8(2, &value, 122);
    EXPECT_EQ(-1, ret);

    //Case: err
    value  = 0;
    ret = cceph_recv_int8(3, &value, 122);
    EXPECT_EQ(-2, ret);

    detach_func(fname_cceph_recv_from_conn);
}

int MOCK_recv_string_cceph_recv_from_conn_success(int data_fd, void* buf, size_t size, int64_t log_id) {
    static int call_time = -1;
    call_time++;

    EXPECT_EQ(1, data_fd);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ(122, log_id);

    if (call_time == 0) {
        EXPECT_EQ(sizeof(int16_t), size);
        int16_t value = 5;
        memcpy(buf, &value, size);
        return size;
    } else if (call_time == 1) {
        EXPECT_EQ((size_t)5, size);
        char* value = (char*)"cceph";
        memcpy(buf, value, size);
        return size;
    }

    EXPECT_TRUE(0 == 1);
    return -1;
}
int MOCK_recv_string_cceph_recv_from_conn_fail1(int data_fd, void* buf, size_t size, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ(122, log_id);
    EXPECT_EQ(sizeof(int16_t), size);
    return 1;
}
int MOCK_recv_string_cceph_recv_from_conn_fail2(int data_fd, void* buf, size_t size, int64_t log_id) {
    static int call_time = -1;
    call_time++;

    EXPECT_EQ(1, data_fd);
    EXPECT_TRUE(buf != NULL);
    EXPECT_EQ(122, log_id);

    if (call_time == 0) {
        EXPECT_EQ(sizeof(int16_t), size);
        int16_t value = 5;
        memcpy(buf, &value, size);
        return size;
    } else if (call_time == 1) {
        EXPECT_EQ((size_t)5, size);
        return size - 1;
    }

    EXPECT_TRUE(0 == 1);
    return -1;
}
TEST(message_io, recv_string) {
    int16_t length = 0;
    char* buf = NULL;

    //Case: normal
    attach_and_enable_func_lib(fname_cceph_recv_from_conn, (void*)&MOCK_recv_string_cceph_recv_from_conn_success);
    int ret = cceph_recv_string(1, &length, &buf, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(5, length);
    EXPECT_STREQ("cceph", buf);
    detach_func(fname_cceph_recv_from_conn);

    //Case: failed when read length;
    attach_and_enable_func_lib(fname_cceph_recv_from_conn, (void*)&MOCK_recv_string_cceph_recv_from_conn_fail1);
    ret = cceph_recv_string(1, &length, &buf, 122);
    EXPECT_EQ(-1, ret);
    detach_func(fname_cceph_recv_from_conn);

    //Case: failed when read string;
    attach_and_enable_func_lib(fname_cceph_recv_from_conn, (void*)&MOCK_recv_string_cceph_recv_from_conn_fail2);
    ret = cceph_recv_string(1, &length, &buf, 122);
    EXPECT_EQ(-1, ret);
    detach_func(fname_cceph_recv_from_conn);
}

//int send_int8(int fd, int8_t value, int64_t log_id);
ssize_t MOCK_send_int8_send_succ(int socket, const void *buffer, size_t length, int flags) {
    EXPECT_EQ(1, socket);
    EXPECT_NE((void*)NULL, buffer);
    EXPECT_EQ(sizeof(int8_t), length);
    EXPECT_EQ(0, flags);
    return length;
}
ssize_t MOCK_send_int8_send_fail1(int socket, const void *buffer, size_t length, int flags) {
    EXPECT_EQ(1, socket);
    EXPECT_NE((void*)NULL, buffer);
    EXPECT_EQ(sizeof(int8_t), length);
    EXPECT_EQ(0, flags);
    return length - 1;
}
ssize_t MOCK_send_int8_send_fail2(int socket, const void *buffer, size_t length, int flags) {
    EXPECT_EQ(1, socket);
    EXPECT_NE((void*)NULL, buffer);
    EXPECT_EQ(sizeof(int8_t), length);
    EXPECT_EQ(0, flags);
    return -1;
}
TEST(message_io, send_int8) {
    //Case: normal
    attach_and_enable_func(fname_send, (void*)&MOCK_send_int8_send_succ);
    int ret = cceph_send_int8(1, 37, 122);
    EXPECT_EQ(0, ret);
    detach_func(fname_send);

    //Case: incomplete send
    attach_and_enable_func(fname_send, (void*)&MOCK_send_int8_send_fail1);
    ret = cceph_send_int8(1, 37, 122);
    EXPECT_EQ(-1, ret);
    detach_func(fname_send);

    //Case: failed when send
    attach_and_enable_func(fname_send, (void*)&MOCK_send_int8_send_fail2);
    ret = cceph_send_int8(1, 37, 122);
    EXPECT_EQ(-1, ret);
    detach_func(fname_send);
}

ssize_t MOCK_send_string_send_succ(int socket, const void *buffer, size_t length, int flags) {
    static int call_time = -1;
    call_time++;

    EXPECT_EQ(1, socket);
    EXPECT_TRUE(buffer != NULL);
    EXPECT_EQ(0, flags);

    if (call_time == 0) {
        EXPECT_EQ(sizeof(int16_t), length);
        EXPECT_EQ(5, (*(int16_t*)buffer));
        return length;
    } else if (call_time == 1) {
        EXPECT_EQ((size_t)5, length);
        EXPECT_STREQ((char*)"cceph", (char*)buffer);
        return length;
    }

    EXPECT_TRUE(0 == 1);
    return -1;
}
ssize_t MOCK_send_string_send_fail1(int socket, const void *buffer, size_t length, int flags) {
    EXPECT_EQ(1, socket);
    EXPECT_TRUE(buffer != NULL);
    EXPECT_EQ(0, flags);
    EXPECT_EQ(sizeof(int16_t), length);
    EXPECT_EQ(5, (*(int16_t*)buffer));
    return length - 1;
}
ssize_t MOCK_send_string_send_fail2(int socket, const void *buffer, size_t length, int flags) {
    static int call_time = -1;
    call_time++;

    EXPECT_EQ(1, socket);
    EXPECT_TRUE(buffer != NULL);
    EXPECT_EQ(0, flags);

    if (call_time == 0) {
        EXPECT_EQ(sizeof(int16_t), length);
        EXPECT_EQ(5, (*(int16_t*)buffer));
        return length;
    } else if (call_time == 1) {
        EXPECT_EQ((size_t)5, length);
        EXPECT_STREQ((char*)"cceph", (char*)buffer);
        return length - 1;
    }

    EXPECT_TRUE(0 == 1);
    return -1;
}
TEST(message_io, send_string) {
    //Case: normal
    attach_and_enable_func(fname_send, (void*)&MOCK_send_string_send_succ);
    int ret = cceph_send_string(1, (char*)"cceph", 122);
    EXPECT_EQ(0, ret);
    detach_func(fname_send);

    //Case: failed when read length;
    attach_and_enable_func(fname_send, (void*)&MOCK_send_string_send_fail1);
    ret = cceph_send_string(1, (char*)"cceph", 122);
    EXPECT_EQ(-1, ret);
    detach_func(fname_send);

    //Case: failed when read string;
    attach_and_enable_func(fname_send, (void*)&MOCK_send_string_send_fail2);
    ret = cceph_send_string(1, (char*)"cceph", 122);
    EXPECT_EQ(-1, ret);
    detach_func(fname_send);
}
