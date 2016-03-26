extern "C" {
#include "os/transaction.h"
}

#include "gtest/gtest.h"

TEST(os_transaction, cceph_os_transaction_new) {
    cceph_os_transaction *tran= cceph_os_transaction_new();
    EXPECT_NE((cceph_os_transaction*)NULL, tran);
    EXPECT_NE((cceph_os_transaction_op*)NULL, tran->op_buffer);

    EXPECT_EQ(CCEPH_OS_TRAN_OP_LIST_SIZE, tran->op_buffer_length);
    EXPECT_EQ(0, tran->op_buffer_index);
}
