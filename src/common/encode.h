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
#define CCEPH_DECODE_TYPE(type)                                                        \
inline extern int cceph_decode_##type(                                                 \
        cceph_buffer_reader* reader,                                                   \
        type*                value,                                                    \
        int64_t              log_id) {                                                 \
    return cceph_buffer_reader_read(reader, (char*)value, sizeof(type), log_id);       \
}

CCEPH_ENCODE_TYPE(char);
CCEPH_ENCODE_TYPE(int);

CCEPH_ENCODE_TYPE(int8_t);
CCEPH_ENCODE_TYPE(int16_t);
CCEPH_ENCODE_TYPE(int32_t);
CCEPH_ENCODE_TYPE(int64_t);

CCEPH_DECODE_TYPE(char);
CCEPH_DECODE_TYPE(int);

CCEPH_DECODE_TYPE(int8_t);
CCEPH_DECODE_TYPE(int16_t);
CCEPH_DECODE_TYPE(int32_t);
CCEPH_DECODE_TYPE(int64_t);

#define cceph_encode_version(buffer, value, log_id) \
        cceph_encode_int8_t(buffer, value, log_id)

#define cceph_decode_version(reader, value, log_id) \
        cceph_decode_int8_t(reader, value, log_id)

#define CCEPH_DEFINE_ENCODE_METHOD(type) \
extern int cceph_encode_##type(          \
        cceph_buffer* buffer,            \
        cceph_##type* value,             \
        int64_t       log_id);           \
extern int cceph_decode_##type(          \
        cceph_buffer_reader* reader,     \
        cceph_##type*        value,      \
        int64_t              log_id);    \


#endif
