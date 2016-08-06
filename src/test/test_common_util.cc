extern "C" {
#include "common/util.h"
}

#include "gtest/gtest.h"

TEST(common_util, intcmp) {
    EXPECT_EQ(-1, intcmp(1, 2));
    EXPECT_EQ(1, intcmp(2, 1));
    EXPECT_EQ(0, intcmp(1, 1));
}



