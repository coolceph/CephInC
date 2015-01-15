extern "C" {
#include "librados/librados.h"
}

#include "gtest/gtest.h"

//This is the unittest for librados
//we should use null_client or mem_client to run this

TEST(librados, Create) {
  rados_t cluster;
  int ret = rados_create(&cluster);
  EXPECT_EQ(ret, 0);
}
