#ifndef CCEPH_TEST_COMMON_H
#define CCEPH_TEST_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int is_full_check(const char* test_name) {
    char *env_cceph_full_check;
    env_cceph_full_check = getenv("CCEPH_FULL_CHECK");
    if (env_cceph_full_check == NULL || strcmp(env_cceph_full_check, "1") != 0) {
        printf("NOT FULL CHECK, test %s will be ignored.\n", test_name);
        return 0;
    } else {
        printf("FULL CHECK, test %s will run.\n", test_name);
        return 1;
    }
    return 1;
}

#endif
