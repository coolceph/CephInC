extern "C" {
#include "common/errno.h"
#include "os/transaction.h"
}

#include "gtest/gtest.h"

TEST(os_transaction, cceph_os_transaction_new) {
    cceph_os_transaction *tran = NULL;
    int ret = cceph_os_transaction_new(&tran, 0);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_os_transaction*)NULL, tran);
    EXPECT_NE((cceph_os_transaction_op*)NULL, tran->op_buffer);

    EXPECT_EQ(CCEPH_OS_TRAN_OP_LIST_SIZE, tran->op_buffer_length);
    EXPECT_EQ(0, tran->op_buffer_index);
}
TEST(os_transaction, cceph_os_write) {
    cceph_os_transaction *tran = NULL;
    int ret = cceph_os_transaction_new(&tran, 0);
    EXPECT_EQ(CCEPH_OK, ret);

    cceph_os_coll_id_t cid = 1;
    const char* oid = "oid";
    int64_t length = 1024;
    int64_t offset  = 0;
    char data[1024];
    int64_t log_id = 122;

    for (int i = 0; i < CCEPH_OS_TRAN_OP_LIST_SIZE * 3; i++) {
        int ret = cceph_os_write(tran,
                cid + i, oid, offset + i, length + i, data, log_id + i);

        EXPECT_EQ(i + 1, cceph_os_tran_get_op_count(tran, log_id));

        int buffer_length = (i + 1) / CCEPH_OS_TRAN_OP_LIST_SIZE;
        buffer_length += (i + 1) % CCEPH_OS_TRAN_OP_LIST_SIZE > 0 ? 1 : 0;
        buffer_length *= CCEPH_OS_TRAN_OP_LIST_SIZE;

        EXPECT_EQ(0, ret);
        EXPECT_EQ(i + 1, tran->op_buffer_index);
        EXPECT_EQ(buffer_length, tran->op_buffer_length);

        cceph_os_transaction_op *op = tran->op_buffer + i;
        EXPECT_EQ(CCEPH_OS_OP_WRITE , op->op);
        EXPECT_EQ(cid + i , op->cid);
        EXPECT_EQ(offset + i , op->offset);
        EXPECT_EQ(length + i , op->length);
        EXPECT_EQ(log_id + i , op->log_id);
        EXPECT_EQ(data , op->data);
        EXPECT_STREQ(oid , op->oid);
    }

    for (int i = 0; i < CCEPH_OS_TRAN_OP_LIST_SIZE * 3; i++) {
        cceph_os_transaction_op *op = cceph_os_tran_get_op(tran, i, log_id);
        EXPECT_EQ(CCEPH_OS_OP_WRITE , op->op);
        EXPECT_EQ(cid + i , op->cid);
        EXPECT_EQ(offset + i , op->offset);
        EXPECT_EQ(length + i , op->length);
        EXPECT_EQ(log_id + i , op->log_id);
        EXPECT_EQ(data , op->data);
        EXPECT_STREQ(oid , op->oid);
    }
}
