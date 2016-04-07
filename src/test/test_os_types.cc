extern "C" {
#include "os/types.h"
}

#include "gtest/gtest.h"

TEST(os_coll_id, cmp) {
    EXPECT_EQ(-1, cceph_os_coll_id_cmp(1, 2));
    EXPECT_EQ(1, cceph_os_coll_id_cmp(3, 2));
    EXPECT_EQ(0, cceph_os_coll_id_cmp(2, 2));
}
