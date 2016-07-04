extern "C" {
#include "common/osdmap.h"

#include "common/buffer.h"
#include "common/errno.h"
#include "common/encode.h"
}

#include "gtest/gtest.h"

TEST(cceph_osd_entity, encode_and_decode) {
    cceph_buffer*        buffer = NULL;
    cceph_buffer_reader* reader = NULL;
    cceph_osd_entity value;
    cceph_osd_entity result;
    int64_t log_id = 122;

    value.id = 1;
    result.id = 0;

    int ret = cceph_buffer_new(&buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_encode_osd_entity(buffer, &value, log_id);
    EXPECT_EQ(CCEPH_OK, ret);

    ret = cceph_buffer_reader_new(&reader, buffer, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    ret = cceph_decode_osd_entity(reader, &result, log_id);
    EXPECT_EQ(CCEPH_OK, ret);
    EXPECT_EQ(value.id, result.id);
}
