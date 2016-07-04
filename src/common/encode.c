#include "common/buffer.h"

#include <string.h>

#include "common/assert.h"
#include "common/encode.h"
#include "common/errno.h"

#define CCEPH_ENCODE_BASIC_TYPE(type)                                                  \
extern int cceph_encode_##type(                                                        \
        cceph_buffer* buffer,                                                          \
        type          value,                                                           \
        int64_t       log_id) {                                                        \
    return cceph_buffer_append(buffer, (char*)&value, sizeof(type), log_id);           \
}                                                                                      \
extern int cceph_decode_##type(                                                        \
        cceph_buffer_reader* reader,                                                   \
        type*                value,                                                    \
        int64_t              log_id) {                                                 \
    return cceph_buffer_reader_read(reader, (char*)value, sizeof(type), log_id);       \
}

CCEPH_ENCODE_BASIC_TYPE(char);
CCEPH_ENCODE_BASIC_TYPE(int);

CCEPH_ENCODE_BASIC_TYPE(int8_t);
CCEPH_ENCODE_BASIC_TYPE(int16_t);
CCEPH_ENCODE_BASIC_TYPE(int32_t);
CCEPH_ENCODE_BASIC_TYPE(int64_t);

int cceph_encode_string(
        cceph_buffer* buffer,
        char*         value,
        int64_t       log_id) {

    assert(log_id, buffer != NULL);
    assert(log_id, value != NULL);

    int length = strlen(value);
    int ret = cceph_buffer_append(buffer, (char*)&length, sizeof(length), log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    return cceph_buffer_append(buffer, value, length, log_id);
}
int cceph_decode_string(
        cceph_buffer_reader* reader,
        char**               value,
        int64_t              log_id) {

    assert(log_id, reader != NULL);
    assert(log_id, value != NULL);
    assert(log_id, *value == NULL);

    int length = 0;
    int ret = cceph_buffer_reader_read(reader, (char*)&length, sizeof(length), log_id);
    if (ret != CCEPH_OK) {
        return ret;
    }

    *value = (char*)malloc(sizeof(char) * length);
    if (*value == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    ret = cceph_buffer_reader_read(reader, *value, length, log_id);
    return ret;
}
