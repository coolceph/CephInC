#include "osdmap.h"

#include "common/assert.h"
#include "common/errno.h"
#include "common/encode.h"

int cceph_encode_osd_entity(
        cceph_buffer*     buffer,
        cceph_osd_entity* osd_entity,
        int64_t           log_id) {

    assert(log_id, buffer != NULL);
    assert(log_id, osd_entity != NULL);

    cceph_version_t v = 1;
    cceph_encode_version(buffer, v, log_id);
    cceph_encode_int32(buffer, osd_entity->id, log_id);

    return CCEPH_OK;
}

int cceph_decode_osd_entity(
        cceph_buffer_reader* reader,
        cceph_osd_entity*    osd_entity,
        int64_t              log_id) {

    assert(log_id, reader != NULL);
    assert(log_id, osd_entity != NULL);

    cceph_version_t v = 0;
    cceph_decode_version(reader, &v, log_id);
    assert(log_id, v == 1);

    cceph_decode_int32(reader, &osd_entity->id, log_id);

    return CCEPH_OK;
}

int cceph_encode_osdmap(
        cceph_buffer* buffer,
        cceph_osdmap* osdmap,
        int64_t       log_id) {

    assert(log_id, buffer != NULL);
    assert(log_id, osdmap != NULL);

    cceph_version_t v = 1;
    cceph_encode_version(buffer, v, log_id);

    cceph_encode_epoch(buffer, osdmap->epoch, log_id);
    cceph_encode_int(buffer, osdmap->pg_count, log_id);

    cceph_encode_int(buffer, osdmap->osd_count, log_id);
    int i = 0;
    for (i = 0; i < osdmap->osd_count; i++) {
        cceph_encode_osd_entity(buffer, &osdmap->osds[i], log_id);
    }

    return CCEPH_OK;
}

int cceph_decode_osdmap(
        cceph_buffer_reader* reader,
        cceph_osdmap*        osdmap,
        int64_t              log_id) {

    assert(log_id, reader != NULL);
    assert(log_id, osdmap != NULL);

    cceph_version_t v = 0;
    cceph_decode_version(reader, &v, log_id);
    assert(log_id, v == 1);

    cceph_decode_epoch(reader, &osdmap->epoch, log_id);
    cceph_decode_int(reader, &osdmap->pg_count, log_id);
    cceph_decode_int(reader, &osdmap->osd_count, log_id);

    osdmap->osds = (cceph_osd_entity*)malloc(sizeof(cceph_osd_entity) * osdmap->osd_count);
    if (osdmap->osds == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    int i = 0;
    for (i = 0; i < osdmap->osd_count; i++) {
        cceph_decode_osd_entity(reader, &osdmap->osds[i], log_id);
    }

    return CCEPH_OK;
}

