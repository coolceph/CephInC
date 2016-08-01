#ifndef CCEPH_OSD_CREATE_H
#define CCEPH_OSD_CREATE_H

#include "common/types.h"
#include "common/osdmap.h"

#include "os/object_store.h"

#include "osd/osd.h"

extern int cceph_osd_create(
        cceph_osd* osd,
        int64_t    log_id);

#endif
