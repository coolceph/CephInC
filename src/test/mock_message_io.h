#ifndef CCEPH_TEST_MOCK_MESSAGE_IO_
#define CCEPH_TEST_MOCK_MESSAGE_IO_

#include "message/io.h"

#include "gtest/gtest.h"

char* cceph_string = (char*)"cceph_string";
char* cceph_data = (char*)"cceph_data";

char* fname_cceph_recv_int8 = (char*)"cceph_recv_int8";
char* fname_cceph_recv_int16 = (char*)"cceph_recv_int16";
char* fname_cceph_recv_int32 = (char*)"cceph_recv_int32";
char* fname_cceph_recv_int64 = (char*)"cceph_recv_int64";
char* fname_cceph_recv_string = (char*)"cceph_recv_string";
char* fname_cceph_recv_data = (char*)"cceph_recv_data";

char* fname_cceph_send_int8 = (char*)"cceph_send_int8";
char* fname_cceph_send_int16 = (char*)"cceph_send_int16";
char* fname_cceph_send_int32 = (char*)"cceph_send_int32";
char* fname_cceph_send_int64 = (char*)"cceph_send_int64";
char* fname_cceph_send_string = (char*)"cceph_send_string";
char* fname_cceph_send_data = (char*)"cceph_send_data";

int MOCK_cceph_recv_int8(int data_fd, int8_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 8;
    return 0;
}
int MOCK_cceph_recv_int16(int data_fd, int16_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 16;
    return 0;
}
int MOCK_cceph_recv_int32(int data_fd, int32_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 32;
    return 0;
}
int MOCK_cceph_recv_int64(int data_fd, int64_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 64;
    return 0;
}
int MOCK_cceph_recv_string(int data_fd, int16_t* length, char** value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = cceph_string;
    *length = strlen(cceph_string);
    return 0;
}
int MOCK_cceph_recv_data(int data_fd, int64_t* length, char** value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = cceph_data;
    *length = strlen(cceph_data);
    return 0;
}

int MOCK_cceph_send_int8(int fd, int8_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(8, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_cceph_send_int16(int fd, int16_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(16, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_cceph_send_int32(int fd, int32_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(32, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_cceph_send_int64(int fd, int64_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(64, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_cceph_send_string(int fd, char* value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_STREQ(value, cceph_string);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_cceph_send_data(int fd, int64_t length, char* value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(strlen(cceph_data), length);
    EXPECT_EQ(0, strncmp(value, cceph_data, strlen(cceph_data)));
    EXPECT_EQ(122, log_id);
    return 0;
}

void attach_message_io_funcs() {
    attach_and_enable_func_lib(fname_cceph_recv_int8, (void*)&MOCK_cceph_recv_int8);
    attach_and_enable_func_lib(fname_cceph_recv_int16, (void*)&MOCK_cceph_recv_int16);
    attach_and_enable_func_lib(fname_cceph_recv_int32, (void*)&MOCK_cceph_recv_int32);
    attach_and_enable_func_lib(fname_cceph_recv_int64, (void*)&MOCK_cceph_recv_int64);
    attach_and_enable_func_lib(fname_cceph_recv_string, (void*)&MOCK_cceph_recv_string);
    attach_and_enable_func_lib(fname_cceph_recv_data, (void*)&MOCK_cceph_recv_data);

    attach_and_enable_func_lib(fname_cceph_send_int8, (void*)&MOCK_cceph_send_int8);
    attach_and_enable_func_lib(fname_cceph_send_int16, (void*)&MOCK_cceph_send_int16);
    attach_and_enable_func_lib(fname_cceph_send_int32, (void*)&MOCK_cceph_send_int32);
    attach_and_enable_func_lib(fname_cceph_send_int64, (void*)&MOCK_cceph_send_int64);
    attach_and_enable_func_lib(fname_cceph_send_string, (void*)&MOCK_cceph_send_string);
    attach_and_enable_func_lib(fname_cceph_send_data, (void*)&MOCK_cceph_send_data);
}
void detach_message_io_funcs() {
    detach_func_lib(fname_cceph_recv_int8);
    detach_func_lib(fname_cceph_recv_int16);
    detach_func_lib(fname_cceph_recv_int32);
    detach_func_lib(fname_cceph_recv_int64);
    detach_func_lib(fname_cceph_recv_string);
    detach_func_lib(fname_cceph_recv_data);

    detach_func_lib(fname_cceph_send_int8);
    detach_func_lib(fname_cceph_send_int16);
    detach_func_lib(fname_cceph_send_int32);
    detach_func_lib(fname_cceph_send_int64);
    detach_func_lib(fname_cceph_send_string);
    detach_func_lib(fname_cceph_send_data);
}

#endif
