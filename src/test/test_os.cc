//This file can't be used individually, it is should be included
//by a concrete ObjectStore such as MemStore.
//
//You can see test_os_mem_store as an example.

TEST_F(os, coll_create_and_remove) {
    int64_t log_id = 122;
    cceph_os_coll_id_t cid = 1;
    cceph_os_transaction *tran = NULL;

    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_create_coll(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Already Exist
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_create_coll(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_ALREADY_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Remove Collection: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_remove_coll(tran, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Remove Collection: Not Exist
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_remove_coll(tran, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
}

TEST_F(os, object_touch_and_remove) {
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
    ret = cceph_os_touch(tran, cid, oid, log_id);
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

    //Read Object
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Touch Object: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_touch(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Touch Object: Already Exist
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_touch(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    result_buffer = NULL;
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(0, result_length);

    //Remove Object: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_remove(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    result_buffer = NULL;
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Remove Object
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_remove(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
}

TEST_F(os, object_write_and_read) {
    int64_t               log_id        = 122;
    cceph_os_coll_id_t    cid           = 1;
    const char*           oid           = "object";
    cceph_os_transaction* tran          = NULL;
    const char*           buffer        = "buffer_content";
    int64_t               offset        = 0;
    int64_t               length        = strlen(buffer);
    int64_t               result_length = 0;
    char*                 result_buffer = NULL;

    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Collection Not Existed
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_write(tran, cid, oid, offset, length, buffer, log_id);
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

    //Read Object
    result_buffer = NULL;
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Write Object: Offset = 0
    buffer = "cceph_buffer_content";
    length = strlen(buffer);
    offset = 0;
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_write(tran, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object: All Content
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(length, result_length);
    EXPECT_STREQ(buffer, result_buffer);

    //Remove Object
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_remove(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Write Object: Offset = 0
    buffer = "cceph_buffer_content";
    length = strlen(buffer);
    offset = 7;
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_write(tran, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object: All Content
    result_buffer = NULL;
    ret = funcs->read(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(offset + length, result_length);
    EXPECT_EQ(0, memcmp("\0\0\0\0\0\0\0cceph_buffer_content", result_buffer, result_length));
}
