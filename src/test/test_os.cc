TEST_F(os, create_and_remove_coll) {
    int64_t log_id = 122;
    cceph_os_coll_id_t cid = 1;
    cceph_os_transaction *tran = NULL;

    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_create_coll(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Remove Collection
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_remove_coll(tran, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
}

TEST_F(os, object_touch) {
    int64_t               log_id        = 122;
    cceph_os_coll_id_t    cid           = 1;
    const char*           oid           = "object";
    cceph_os_transaction* tran          = NULL;
    int64_t               result_length = 0;
    char*                 result_buffer = NULL;

    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Collection Not Existed
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_write(tran, cid, oid, 0, 0, NULL, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_create_coll(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Touch Object
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_touch(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(0, result_length);
}
