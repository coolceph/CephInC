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
    ret = cceph_os_coll_create(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Already Exist
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_coll_create(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_ALREADY_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Remove Collection: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_coll_remove(tran, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Remove Collection: Not Exist
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_coll_remove(tran, cid, log_id);
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
    ret = cceph_os_obj_touch(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_coll_create(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Touch Object: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_touch(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Touch Object: Already Exist
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_touch(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(0, result_length);

    //Remove Object: Success
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_remove(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Remove Object
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_remove(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
}

typedef struct {
    cceph_object_store*   os;
    cceph_os_funcs*       funcs;
    int64_t               log_id;
    cceph_os_coll_id_t    cid;
    const char*           oid;
} write_read_thread_arg;

void* write_read_thread_func(void* arg_ptr) {

    write_read_thread_arg *arg = (write_read_thread_arg*)arg_ptr;

    cceph_object_store*   os     = arg->os;
    cceph_os_funcs*       funcs  = arg->funcs;
    int64_t               log_id = arg->log_id;
    cceph_os_coll_id_t    cid    = arg->cid;
    const char*           oid    = arg->oid;

    cceph_os_transaction* tran          = NULL;
    const char*           buffer        = "buffer_content";
    int64_t               offset        = 0;
    int64_t               length        = strlen(buffer);
    int64_t               result_length = 0;
    char*                 result_buffer = NULL;

    //Read Object
    result_buffer = NULL;
    int ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Write Object: Offset = 0
    buffer = "cceph_buffer_content";
    length = strlen(buffer);
    offset = 0;
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_write(tran, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object: All Content
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(length, result_length);
    EXPECT_STREQ(buffer, result_buffer);

    //Remove Object
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_remove(tran, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Write Object: Offset = 7
    buffer = "cceph_buffer_content";
    length = strlen(buffer);
    offset = 7;
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_write(tran, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object: All Content
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(offset + length, result_length);
    EXPECT_EQ(0, memcmp("\0\0\0\0\0\0\0cceph_buffer_content", result_buffer, result_length));

    //Write Object: Offset = 0
    buffer = "cceph_buffer_content";
    length = 7;
    offset = 0;
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_write(tran, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object: All Content
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(7 + strlen(buffer), result_length);
    EXPECT_EQ(0, memcmp("cceph_bcceph_buffer_content", result_buffer, result_length));

    return NULL;
}

TEST_F(os, object_write_and_read) {
    int64_t log_id = 122;
    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    write_read_thread_arg arg;
    arg.os     = os;
    arg.funcs  = funcs;
    arg.log_id = log_id;
    arg.cid    = 1;
    arg.oid    = "object";

    cceph_os_transaction* tran          = NULL;

    //Collection Not Existed
    cceph_os_coll_id_t    cid           = 1;
    const char*           oid           = "object";
    const char*           buffer        = "buffer_content";
    int64_t               offset        = 0;
    int64_t               length        = strlen(buffer);
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_obj_write(tran, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection
    ret = cceph_os_transaction_new(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_coll_create(tran, cid,  log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = funcs->submit_transaction(os, tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_transaction_free(&tran, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    write_read_thread_func(&arg);
}
TEST_F(os, object_write_and_read_multithread) {
    int64_t               log_id = 122;
    cceph_object_store*   os     = GetObjectStore(log_id);
    cceph_os_funcs*       funcs  = GetObjectStoreFuncs();
    cceph_os_transaction* tran   = NULL;

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int thread_count = 64;
    pthread_t thread_ids[thread_count];
    for (int i = 0; i < thread_count; i++) {

        //Create Collection
        ret = cceph_os_transaction_new(&tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = cceph_os_coll_create(tran, i,  log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = funcs->submit_transaction(os, tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = cceph_os_transaction_free(&tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);

        char* oid = (char*)malloc(sizeof(char) * 256);
        bzero(oid, 256);
        sprintf(oid, "%d", i);

        write_read_thread_arg *arg = (write_read_thread_arg*)malloc(sizeof(write_read_thread_arg));
        arg->os     = os;
        arg->funcs  = funcs;
        arg->log_id = log_id + i;
        arg->cid    = i % 4;
        arg->oid    = oid;
        int ret = pthread_create(thread_ids + i, &thread_attr, &write_read_thread_func, arg);
        EXPECT_EQ(0, ret);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(*(thread_ids + i), NULL);
    }
    for (int i = 0; i < thread_count; i++) {
        //Remove Collection: Success
        ret = cceph_os_transaction_new(&tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = cceph_os_coll_remove(tran, i, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = funcs->submit_transaction(os, tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = cceph_os_transaction_free(&tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);

        //Remove Collection: Not Exist
        ret = cceph_os_transaction_new(&tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = cceph_os_coll_remove(tran, i, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        ret = funcs->submit_transaction(os, tran, log_id);
        EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
        ret = cceph_os_transaction_free(&tran, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
    }
}
