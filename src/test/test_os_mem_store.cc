extern "C" {
#include "common/errno.h"
#include "os/mem_store.h"
}

#include "gtest/gtest.h"

TEST(os_mem_store, cceph_mem_store_coll_node_new_and_free) {
    int64_t                   log_id = 122;
    cceph_os_coll_id_t        cid    = 1;
    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_new(cid, &cnode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store_coll_node*)NULL, cnode);
    EXPECT_EQ(cid, cnode->cid);

    ret = cceph_mem_store_coll_node_free(&cnode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_mem_store_coll_node*)NULL, cnode);
}
TEST(os_mem_store, cceph_mem_store_object_node_new_and_free) {
    int64_t                   log_id = 122;
    const char*               oid    = "oid";
    cceph_mem_store_object_node *onode = NULL;
    int ret = cceph_mem_store_object_node_new(oid, &onode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store_object_node*)NULL, onode);
    EXPECT_STREQ(oid, onode->oid);

    ret = cceph_mem_store_object_node_free(&onode, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ((cceph_mem_store_object_node*)NULL, onode);
}

TEST(os_mem_store, cceph_mem_store_new) {
    cceph_mem_store *store = NULL;
    int ret = cceph_mem_store_new(&store, 0);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_NE((cceph_mem_store*)NULL, store);
}
TEST(os_mem_store, cceph_mem_store_get_funcs) {
    cceph_os_funcs *funcs = cceph_mem_store_get_funcs();

    EXPECT_TRUE(funcs->mount             == cceph_mem_store_mount);
    EXPECT_TRUE(funcs->submit_tran       == cceph_mem_store_submit_tran);

    EXPECT_TRUE(funcs->list_coll         == cceph_mem_store_list_coll);

    EXPECT_TRUE(funcs->read_obj          == cceph_mem_store_read_obj);

    EXPECT_TRUE(funcs->read_coll_map     == cceph_mem_store_read_coll_map);
    EXPECT_TRUE(funcs->read_coll_map_key == cceph_mem_store_read_coll_map_key);

    EXPECT_TRUE(funcs->read_obj_map      == cceph_mem_store_read_obj_map);
    EXPECT_TRUE(funcs->read_obj_map_key  == cceph_mem_store_read_obj_map_key);
}

class os : public ::testing::Test {
public:
    cceph_object_store* GetObjectStore(int64_t log_id) {
        cceph_mem_store *store = NULL;
        int ret = cceph_mem_store_new(&store, log_id);
        EXPECT_EQ(CCEPH_OK, ret);
        return (cceph_object_store*)store;
    }
    cceph_os_funcs* GetObjectStoreFuncs() {
        return cceph_mem_store_get_funcs();
    }
};

#include "test/test_os.cc"
