#include "pg.h"

#include "common/assert.h"
#include "common/errno.h"

int cceph_pg_new(
        cceph_pg**    pg,
        cceph_pg_id_t pg_id,
        int64_t       log_id) {

    assert(log_id, pg != NULL);
    assert(log_id, *pg == NULL);

    *pg = (cceph_pg*)malloc(sizeof(cceph_pg));
    if (*pg == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*pg)->pg_id = pg_id;
    (*pg)->state = CCEPH_PG_STATE_UNKNOWN;

    pthread_mutex_init(&((*pg)->lock), NULL);

    return CCEPH_OK;
}

int cceph_pg_free(
        cceph_pg** pg,
        int64_t    log_id) {

    assert(log_id, pg != NULL);
    assert(log_id, *pg != NULL);

    free(*pg);

    *pg = NULL;

    return CCEPH_OK;
}
