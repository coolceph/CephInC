#ifndef CCEPH_OSD_TYPES_H
#define CCEPH_OSD_TYPES_H

#include "message/messenger.h"
#include "message/server_messenger.h"
#include "message/msg_header.h"

#include "os/object_store.h"

typedef struct {
    cceph_osd_id_t          osd_id;

    cceph_object_store*     os;
    cceph_os_funcs*         os_funcs;

    cceph_messenger*        msger;
    cceph_server_messenger* smsger;
} cceph_osd;

#endif