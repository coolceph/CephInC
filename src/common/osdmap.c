#include "osdmap.h"

#include "common/assert.h"
#include "common/errno.h"
#include "common/encode.h"

extern int cceph_encode_osd_entity(
        cceph_buffer*     buffer,
        cceph_osd_entity* osd_entity,
        int64_t           log_id) {

    cceph_version_t v = 1;
    cceph_encode_version(buffer, v, log_id);
    cceph_encode_int32_t(buffer, osd_entity->id, log_id);

    return CCEPH_OK;
}

extern int cceph_decode_osd_entity(
        cceph_buffer_reader* reader,
        cceph_osd_entity*    osd_entity,
        int64_t              log_id) {

    cceph_version_t v = 0;
    cceph_decode_version(reader, &v, log_id);
    assert(log_id, v == 1);

    cceph_decode_int32_t(reader, &osd_entity->id, log_id);

    return CCEPH_OK;
}

