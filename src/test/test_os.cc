TEST_F(os, write_and_read) {
    int64_t log_id = 122;
    cceph_object_store *os    = GetObjectStore(log_id);
    cceph_os_funcs     *funcs = GetObjectStoreFuncs();

    int ret = funcs->mount(os, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
}
