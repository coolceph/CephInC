#ifndef CCEPH_ENCODE_H
#define CCEPH_ENCODE_H

#include "common/buffer.h"

#define CCEPH_ENCODE_TYPE(type)                                                 \
inline extern int cceph_encode_##type(                                          \
        cceph_buffer* buffer,                                                   \
        type          value,                                                    \
        int64_t       log_id) {                                                 \
    return cceph_buffer_append(buffer, (char*)&value, sizeof(type), log_id);    \
}

CCEPH_ENCODE_TYPE(char);
CCEPH_ENCODE_TYPE(int);

CCEPH_ENCODE_TYPE(int8_t);
CCEPH_ENCODE_TYPE(int16_t);
CCEPH_ENCODE_TYPE(int32_t);
CCEPH_ENCODE_TYPE(int64_t);

#endif
