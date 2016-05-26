#ifndef CCEPH_OBJECT_STORE_H
#define CCEPH_OBJECT_STORE_H

#include "common/types.h"

#include "os/types.h"
#include "os/transaction.h"

typedef void* cceph_object_store;

typedef int (*cceph_os_mount_func)(
        cceph_object_store* os,
        int64_t             log_id);

typedef int (*cceph_os_submit_tran_func)(
        cceph_object_store*   os,
        cceph_os_tran*        tran,
        int64_t               log_id);

typedef int (*cceph_os_list_coll_func)(
        cceph_object_store*  os,
        int32_t*             coll_id_list_length,
        cceph_os_coll_id_t** coll_id_list,
        int64_t              log_id);

//if length <= 0 or length >= object->length, read the whole content
typedef int (*cceph_os_read_obj_func)(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        int64_t*            result_length,
        char**              result_data,
        int64_t             log_id);

typedef int (*cceph_os_read_coll_map_func)(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        cceph_rb_root*      map,
        int64_t             log_id);

typedef int (*cceph_os_read_coll_map_key_func)(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         key,
        int32_t*            result_value_length,
        char**              result_value,
        int64_t             log_id);

typedef int (*cceph_os_read_obj_map_func)(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        cceph_rb_root*      map,
        int64_t             log_id);

typedef int (*cceph_os_read_obj_map_key_func)(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        const char*         key,
        int32_t*            result_value_length,
        char**              result_value,
        int64_t             log_id);

typedef struct {
    cceph_os_mount_func               mount;
    cceph_os_submit_tran_func         submit_tran;

    cceph_os_list_coll_func           list_coll;

    cceph_os_read_coll_map_func       read_coll_map;
    cceph_os_read_coll_map_key_func   read_coll_map_key;

    cceph_os_read_obj_func            read_obj;
    cceph_os_read_obj_map_func        read_obj_map;
    cceph_os_read_obj_map_key_func    read_obj_map_key;
} cceph_os_funcs;

#endif
