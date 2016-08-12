#ifndef CCEPH_OSD_MSG_PROCESS_H
#define CCEPH_OSD_MSG_PROCESS_H

#include "message/messenger.h"
#include "message/server_messenger.h"
#include "message/msg_header.h"

#include "os/object_store.h"

#include "osd/types.h"

extern int cceph_osd_process_message(
        cceph_messenger* cceph_messenger,
        cceph_conn_id_t conn_id,
        cceph_msg_header* message,
        void* context);

#endif
