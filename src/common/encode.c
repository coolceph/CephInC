#include "common/buffer.h"
#include "common/encode.h"

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
