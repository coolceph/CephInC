extern "C" {
#include "client/client.h"
}

#include "gtest/gtest.h"

TEST(libclient, write_obj) {
    struct osdmap osdmap;
    osdmap.osd_count = 3;
    osdmap.osds = (struct osd*)malloc(sizeof(struct osd) * 3);
    osdmap.osds[0].host = (char*)"127.0.0.1";
    osdmap.osds[0].port = 9000;
    osdmap.osds[1].host = (char*)"127.0.0.1";
    osdmap.osds[1].port = 9001;
    osdmap.osds[2].host = (char*)"127.0.0.1";
    osdmap.osds[2].port = 9002;

    char*   oid = (char*)"test_oid";
    int64_t offset = 0;
    int64_t length = 4096;
    char    data[4096];
    
    EXPECT_EQ(client_write_obj(&osdmap, oid, offset, length, data), 0);
}
