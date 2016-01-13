extern "C" {
#include "message/msg_header.h"
}

#include "bhook.h"
#include "gtest/gtest.h"

char* lib_func_name_recv_int8 = (char*)"recv_int8";
char* lib_func_name_recv_int64 = (char*)"recv_int64";
char* lib_func_name_send_int8 = (char*)"send_int8";
char* lib_func_name_send_int64 = (char*)"send_int64";

int MOCK_recv_msg_header__recv_int8(int data_fd, int8_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 1;
    return 0;
}
int MOCK_recv_msg_header__recv_int64(int data_fd, int64_t* value, int64_t log_id) {
    EXPECT_EQ(1, data_fd);
    EXPECT_EQ(122, log_id);
    *value = 222;
    return 0;
}
TEST(message_msg_header, recv_msg_header) {
    msg_header header;
    header.op = 0; header.log_id = 0;

    attach_and_enable_func_lib(lib_func_name_recv_int8, (void*)&MOCK_recv_msg_header__recv_int8);
    attach_and_enable_func_lib(lib_func_name_recv_int64, (void*)&MOCK_recv_msg_header__recv_int64);

    int ret = recv_msg_header(1, &header, 122);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1, header.op);
    EXPECT_EQ(222, header.log_id);

    detach_func_lib(lib_func_name_recv_int8);
    detach_func_lib(lib_func_name_recv_int64);
}

int MOCK_send_msg_header__send_int8(int fd, int8_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(1, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
int MOCK_send_msg_header__send_int64(int fd, int64_t value, int64_t log_id) {
    EXPECT_EQ(1, fd);
    EXPECT_EQ(222, value);
    EXPECT_EQ(122, log_id);
    return 0;
}
TEST(message_msg_header, send_msg_header) {
    msg_header header;
    header.op = 1; header.log_id = 222;

    attach_and_enable_func_lib(lib_func_name_send_int8, (void*)&MOCK_send_msg_header__send_int8);
    attach_and_enable_func_lib(lib_func_name_send_int64, (void*)&MOCK_send_msg_header__send_int64);

    int ret = send_msg_header(1, &header, 122);
    EXPECT_EQ(0, ret);

    detach_func_lib(lib_func_name_send_int8);
    detach_func_lib(lib_func_name_send_int64);
}

TEST(message_msg_header, free_msg_header) {
    msg_header *header = (msg_header*)malloc(sizeof(msg_header));
    free_msg_header(&header, 122);
    EXPECT_EQ((msg_header*)NULL, header);
}
