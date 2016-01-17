#ifndef CCEPH_TEST_MOCK_MESSAGE_IO_
#define CCEPH_TEST_MOCK_MESSAGE_IO_

#include "message/io.h"

#include "gtest/gtest.h"

char* fname_recv_int8 = (char*)"recv_int8";
char* fname_recv_int64 = (char*)"recv_int64";
char* fname_send_int8 = (char*)"send_int8";
char* fname_send_int64 = (char*)"send_int64";

int MOCK_recv_int8(int data_fd, int8_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 8;
    return 0;
}
int MOCK_recv_int64(int data_fd, int64_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 64;
    return 0;
}
int MOCK_send_int8(int fd, int8_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(8, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_send_int64(int fd, int64_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(64, value);
    EXPECT_EQ(122, log_id);
    return 0;
}

void attach_message_io_funcs() {
    attach_and_enable_func_lib(fname_recv_int8, (void*)&MOCK_recv_int8);
    attach_and_enable_func_lib(fname_recv_int64, (void*)&MOCK_recv_int64);

    attach_and_enable_func_lib(fname_send_int8, (void*)&MOCK_send_int8);
    attach_and_enable_func_lib(fname_send_int64, (void*)&MOCK_send_int64);
}
void detach_message_io_funcs() {
    detach_func_lib(fname_recv_int8);
    detach_func_lib(fname_recv_int64);

    detach_func_lib(fname_send_int8);
    detach_func_lib(fname_send_int64);
}

#endif
