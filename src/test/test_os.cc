//This file can't be used individually, it is should be included
//by a concrete ObjectStore such as MemStore.
//
//You can see test_os_mem_store as an example.

TEST_F(os, coll_create_and_remove) {
    int64_t             log_id = 122;
    cceph_os_coll_id_t  cid    = 1;
    cceph_object_store* os     = GetObjectStore(log_id);
    cceph_os_funcs*     funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Success
    ret = cceph_os_create_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Already Exist
    ret = cceph_os_create_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_ALREADY_EXIST, ret);

    //Remove Collection: Success
    ret = cceph_os_remove_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Remove Collection: Not Exist
    ret = cceph_os_remove_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, cceph_os_remove_coll(os, funcs, cid, log_id));
}

TEST_F(os, coll_list) {
    int64_t             log_id = 122;
    cceph_object_store* os     = GetObjectStore(log_id);
    cceph_os_funcs*     funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Success
    ret = cceph_os_create_coll(os, funcs, 1, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_create_coll(os, funcs, 2, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_os_create_coll(os, funcs, 3, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    int coll_list_length = 0;
    cceph_os_coll_id_t* coll_list = NULL;
    ret = funcs->list_coll(os, &coll_list_length, &coll_list, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(3, coll_list_length);
    for (int i = 0; i < coll_list_length; i++) {
        cceph_os_coll_id_t id = coll_list[i];
        EXPECT_TRUE(id == 1 || id == 2 || id == 3);
    }
}

TEST_F(os, coll_exist) {
    int64_t             log_id = 122;
    cceph_object_store* os     = GetObjectStore(log_id);
    cceph_os_funcs*     funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    int8_t is_existed = -1;
    ret = funcs->exist_coll(os, 1, &is_existed, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(0, is_existed);

    //Create Collection: Success
    ret = cceph_os_create_coll(os, funcs, 1, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = funcs->exist_coll(os, 1, &is_existed, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(1, is_existed);
}

TEST_F(os, object_touch_and_remove) {
    int64_t             log_id        = 122;
    cceph_os_coll_id_t  cid           = 1;
    const char*         oid           = "object";
    int64_t             result_length = 0;
    char*               result_buffer = NULL;

    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Collection Not Existed
    ret = cceph_os_touch_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);

    //Create Collection
    ret = cceph_os_create_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Touch Object: Success
    ret = cceph_os_touch_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Touch Object: Already Exist
    ret = cceph_os_touch_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(0, result_length);

    //Remove Object: Success
    ret = cceph_os_remove_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
    EXPECT_EQ(0, result_length);

    //Remove Object
    ret = cceph_os_remove_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_ERR_OBJECT_NOT_EXIST, ret);
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
    ret = cceph_os_write_obj(os, funcs, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Read Object: All Content
    result_buffer = NULL;
    ret = funcs->read_obj(os, cid, oid, 0, -1, &result_length, &result_buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(length, result_length);
    EXPECT_TRUE(strncmp(buffer, result_buffer, length) == 0);

    //Remove Object
    ret = cceph_os_remove_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Write Object: Offset = 7
    buffer = "cceph_buffer_content";
    length = strlen(buffer);
    offset = 7;
    ret = cceph_os_write_obj(os, funcs, cid, oid, offset, length, buffer, log_id);
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
    ret = cceph_os_write_obj(os, funcs, cid, oid, offset, length, buffer, log_id);
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
    int64_t             log_id = 122;
    cceph_object_store* os     = GetObjectStore(log_id);
    cceph_os_funcs*     funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    write_read_thread_arg arg;
    arg.os     = os;
    arg.funcs  = funcs;
    arg.log_id = log_id;
    arg.cid    = 1;
    arg.oid    = "object";

    //Collection Not Existed
    cceph_os_coll_id_t    cid           = 1;
    const char*           oid           = "object";
    const char*           buffer        = "buffer_content";
    int64_t               offset        = 0;
    int64_t               length        = strlen(buffer);
    ret = cceph_os_write_obj(os, funcs, cid, oid, offset, length, buffer, log_id);
    EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);

    //Create Collection
    ret = cceph_os_create_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    write_read_thread_func(&arg);
}
TEST_F(os, object_write_and_read_multithread) {
    int64_t               log_id = 122;
    cceph_object_store*   os     = GetObjectStore(log_id);
    cceph_os_funcs*       funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int thread_count = 64;
    pthread_t thread_ids[thread_count];
    for (int i = 0; i < thread_count; i++) {

        //Create Collection
        ret = cceph_os_create_coll(os, funcs, i, log_id);
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
        ret = cceph_os_remove_coll(os, funcs, i, log_id);
        EXPECT_EQ(CCEPH_OK, ret);

        //Remove Collection: Not Exist
        ret = cceph_os_remove_coll(os, funcs, i, log_id);
        EXPECT_EQ(CCEPH_ERR_COLL_NOT_EXIST, ret);
    }
}
TEST_F(os, coll_map) {
    int64_t             log_id = 122;
    cceph_os_coll_id_t  cid    = 1;
    cceph_object_store* os     = GetObjectStore(log_id);
    cceph_os_funcs*     funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Collection: Success
    ret = cceph_os_create_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Not Exist
    cceph_os_map_node* result_node = NULL;
    cceph_rb_root      result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_coll_map(os, cid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_TRUE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);

    //MapKey: Not Exist
    char*   result_value  = NULL;
    int32_t result_length = 0;
    ret = funcs->read_coll_map_key(os, cid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    EXPECT_EQ((char*)NULL, result_value);
    EXPECT_EQ(0, result_length);

    //Map Add
    ret = cceph_os_set_coll_map_key(os, funcs, cid, "key1", "value1", strlen("value1"), log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Exist, Value = "value1"
    result_node = NULL;
    result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_coll_map(os, cid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_FALSE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1"), result_node->value_length);
    EXPECT_EQ(0, strncmp("value1", result_node->value, result_node->value_length));

    //MapKey: Exist, value = "value1"
    result_value  = NULL;
    result_length = 0;
    ret = funcs->read_coll_map_key(os, cid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1"), result_length);
    EXPECT_EQ(0, strncmp("value1", result_value, result_length));

    //Map Update
    ret = cceph_os_set_coll_map_key(os, funcs, cid, "key1", "value1_changed", strlen("value1_changed"), log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Exist, Value = "value1_changed"
    result_node = NULL;
    result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_coll_map(os, cid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_FALSE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1_changed"), result_node->value_length);
    EXPECT_EQ(0, strncmp("value1_changed", result_node->value, result_node->value_length));

    //MapKey: Exist, value = "value1_changed"
    result_value  = NULL;
    result_length = 0;
    ret = funcs->read_coll_map_key(os, cid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1_changed"), result_length);
    EXPECT_EQ(0, strncmp("value1_changed", result_value, result_length));

    //Map Remove
    //Initial Input Node and map
    ret = cceph_os_set_coll_map_key(os, funcs, cid, "key1", NULL, 0, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Not Exist
    result_node = NULL;
    result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_coll_map(os, cid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_TRUE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);

    //MapKey: Not Exist
    result_value  = NULL;
    result_length = 0;
    ret = funcs->read_coll_map_key(os, cid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    EXPECT_EQ((char*)NULL, result_value);
    EXPECT_EQ(0, result_length);
}
TEST_F(os, obj_map) {
    int64_t             log_id = 122;
    cceph_os_coll_id_t  cid    = 1;
    const char*         oid    = (const char*)"oid";
    cceph_object_store* os     = GetObjectStore(log_id);
    cceph_os_funcs*     funcs  = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Create Coll: Success
    ret = cceph_os_create_coll(os, funcs, cid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Touch Object
    ret = cceph_os_touch_obj(os, funcs, cid, oid, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Not Exist
    cceph_os_map_node* result_node = NULL;
    cceph_rb_root      result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_obj_map(os, cid, oid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_TRUE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);

    //MapKey: Not Exist
    char*   result_value  = NULL;
    int32_t result_length = 0;
    ret = funcs->read_obj_map_key(os, cid, oid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    EXPECT_EQ((char*)NULL, result_value);
    EXPECT_EQ(0, result_length);

    //Map Add
    ret = cceph_os_set_obj_map_key(os, funcs, cid, oid, "key1", "value1", strlen("value1"), log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Exist, Value = "value1"
    result_node = NULL;
    result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_obj_map(os, cid, oid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_FALSE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1"), result_node->value_length);
    EXPECT_EQ(0, strncmp("value1", result_node->value, result_node->value_length));

    //MapKey: Exist, value = "value1"
    result_value  = NULL;
    result_length = 0;
    ret = funcs->read_obj_map_key(os, cid, oid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1"), result_length);
    EXPECT_EQ(0, strncmp("value1", result_value, result_length));

    //Map Update
    ret = cceph_os_set_obj_map_key(os, funcs, cid, oid, "key1", "value1_changed", strlen("value1_changed"), log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Exist, Value = "value1_changed"
    result_node = NULL;
    result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_obj_map(os, cid, oid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_FALSE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1_changed"), result_node->value_length);
    EXPECT_EQ(0, strncmp("value1_changed", result_node->value, result_node->value_length));

    //MapKey: Exist, value = "value1_changed"
    result_value  = NULL;
    result_length = 0;
    ret = funcs->read_obj_map_key(os, cid, oid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(strlen("value1_changed"), result_length);
    EXPECT_EQ(0, strncmp("value1_changed", result_value, result_length));

    //Map Remove
    ret = cceph_os_set_obj_map_key(os, funcs, cid, oid, "key1", NULL, 0, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    //Map: Not Exist
    result_node = NULL;
    result_map  = CCEPH_RB_ROOT;
    ret = funcs->read_obj_map(os, cid, oid, &result_map, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_TRUE(CCEPH_RB_EMPTY_ROOT(&result_map));
    ret = cceph_os_map_node_search(&result_map, "key1", &result_node, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);

    //MapKey: Not Exist
    result_value  = NULL;
    result_length = 0;
    ret = funcs->read_obj_map_key(os, cid, oid, "key1", &result_length, &result_value, log_id);
    EXPECT_EQ(CCEPH_ERR_MAP_NODE_NOT_EXIST, ret);
    EXPECT_EQ((char*)NULL, result_value);
    EXPECT_EQ(0, result_length);
}
