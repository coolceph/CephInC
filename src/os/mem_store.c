#include "mem_store.h"

#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"
#include "common/rbtree.h"
#include "common/types.h"

extern int cceph_mem_store_coll_node_new(
        cceph_os_coll_id_t          cid,
        cceph_mem_store_coll_node** node,
        int64_t                     log_id) {
    assert(log_id, node != NULL);
    assert(log_id, *node == NULL);
    assert(log_id, cid >= 0);

    *node = (cceph_mem_store_coll_node*)malloc(sizeof(cceph_mem_store_coll_node));
    if (*node == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*node)->cid     = cid;
    (*node)->objects = CCEPH_RB_ROOT;
    (*node)->map     = CCEPH_RB_ROOT;

    return CCEPH_OK;
}

extern int cceph_mem_store_coll_node_free(
        cceph_mem_store_coll_node** node,
        int64_t                     log_id) {
    assert(log_id, node != NULL);
    assert(log_id, *node != NULL);

    cceph_mem_store_coll_node* cnode = *node;

    int ret = cceph_mem_store_object_node_map_free(&cnode->objects, log_id);
    assert(log_id, ret == CCEPH_OK);

    ret = cceph_os_map_node_map_free(&cnode->map, log_id);
    assert(log_id, ret == CCEPH_OK);

    free(cnode);
    *node = NULL;
    return CCEPH_OK;
}

CCEPH_IMPL_MAP(mem_store_coll_node, cceph_os_coll_id_t, cid, cceph_os_coll_id_cmp);

int cceph_mem_store_object_node_new(
        const char*                   oid,
        cceph_mem_store_object_node** node,
        int64_t                       log_id) {

    assert(log_id, node != NULL);
    assert(log_id, oid != NULL);

    *node = (cceph_mem_store_object_node*)malloc(sizeof(cceph_mem_store_object_node));
    if (*node == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    int oid_length = strlen(oid) + 1;
    (*node)->oid = (char*)malloc(sizeof(char) * oid_length);
    if ((*node)->oid == NULL) {
        free(*node);
        *node = NULL;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }
    memset((*node)->oid, 0, oid_length);
    strcpy((*node)->oid, oid);

    (*node)->data   = NULL;
    (*node)->length = 0;
    (*node)->map    = CCEPH_RB_ROOT;

    return CCEPH_OK;
}

int cceph_mem_store_object_node_free(
        cceph_mem_store_object_node** node,
        int64_t                       log_id) {

    assert(log_id, node != NULL);
    assert(log_id, *node != NULL);

    cceph_mem_store_object_node* onode = *node;
    if (onode->oid != NULL) {
        free(onode->oid);
        onode->oid = NULL;
    }
    if (onode->data != NULL) {
        free(onode->data);
        onode->data = NULL;
    }

    int ret = cceph_os_map_node_map_free(&onode->map, log_id);
    assert(log_id, ret == CCEPH_OK);

    free(onode);
    *node = NULL;

    return CCEPH_OK;
}

CCEPH_IMPL_MAP(mem_store_object_node, const char*, oid, strcmp);

cceph_os_funcs* cceph_mem_store_get_funcs() {
    cceph_os_funcs *os_funcs = (cceph_os_funcs*)malloc(sizeof(cceph_os_funcs));
    memset(os_funcs, 0, sizeof(cceph_os_funcs));

    os_funcs->mount             = cceph_mem_store_mount;
    os_funcs->submit_tran       = cceph_mem_store_submit_tran;

    os_funcs->list_coll         = cceph_mem_store_list_coll;
    os_funcs->exist_coll        = cceph_mem_store_exist_coll;

    os_funcs->read_coll_map     = cceph_mem_store_read_coll_map;
    os_funcs->read_coll_map_key = cceph_mem_store_read_coll_map_key;

    os_funcs->read_obj          = cceph_mem_store_read_obj;
    os_funcs->read_obj_map      = cceph_mem_store_read_obj_map;
    os_funcs->read_obj_map_key  = cceph_mem_store_read_obj_map_key;

    return os_funcs;
}

int cceph_mem_store_new(
        cceph_mem_store** store,
        int64_t           log_id) {

    assert(log_id, store  != NULL);
    assert(log_id, *store == NULL);

    *store = (cceph_mem_store*)malloc(sizeof(cceph_mem_store));
    if (*store == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    (*store)->colls = CCEPH_RB_ROOT;
    pthread_mutex_init(&((*store)->lock), NULL);
    return CCEPH_OK;
}

int cceph_mem_store_mount(
        cceph_object_store* os,
        int64_t             log_id) {
    //MemStore don't need mount
    assert(log_id, os != NULL);

    LOG(LL_NOTICE, log_id, "MemStore Mounted.");

    return CCEPH_OK;
}

int cceph_mem_store_do_op_coll_create(
        cceph_mem_store*    os,
        cceph_os_tran_op*   op,
        int64_t             log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);
    assert(log_id, op->cid >= 0);

    LOG(LL_INFO, log_id, "Execute CreateCollection op, cid %d, log_id %ld.", op->cid, op->log_id);

    log_id = op->log_id; //We will use op's log_id from here

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&os->colls, op->cid, &cnode, log_id);
    if (ret == CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute CreateCollection op failed, cid %d already existed.", op->cid);
        return CCEPH_ERR_COLL_ALREADY_EXIST;
    }

    assert(log_id, cnode == NULL);
    ret = cceph_mem_store_coll_node_new(op->cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute CreateCollection op failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_mem_store_coll_node_map_insert(&os->colls, cnode, log_id);
    if (ret != CCEPH_OK) {
        cceph_mem_store_coll_node_free(&cnode, log_id);
        LOG(LL_ERROR, log_id, "Execute CreateCollection op failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return ret;
    }

    return CCEPH_OK;
}

int cceph_mem_store_do_op_coll_remove(
        cceph_mem_store*   os,
        cceph_os_tran_op*  op,
        int64_t            log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);
    assert(log_id, op->cid >= 0);

    LOG(LL_INFO, log_id, "Execute RemoveCollection op, cid %d, log_id %ld.", op->cid, op->log_id);

    log_id = op->log_id; //We will use op's log_id from here

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&os->colls, op->cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute RemoveCollection op failed, cid %d not existed.", op->cid);
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    ret = cceph_mem_store_coll_node_map_remove(&os->colls, cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute RemoveCollection op failed, can't remove from mem_store, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_mem_store_coll_node_free(&cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute RemoveCollection op failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return ret;
    }

    return CCEPH_OK;
}

int cceph_mem_store_do_op_coll_map(
        cceph_mem_store*   os,
        cceph_os_tran_op*  op,
        int64_t            log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);
    assert(log_id, op->cid >= 0);
    assert(log_id, op->map != NULL);

    LOG(LL_INFO, log_id, "Execute CollectionMap op, cid %d, log_id %ld.", op->cid, op->log_id);

    log_id = op->log_id; //We will use op's log_id from here

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&os->colls, op->cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute CollectionMap op failed, cid %d not existed.", op->cid);
        return ret;
    }

    ret = cceph_os_map_update(&cnode->map, op->map, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute CollectionMap op failed, update coll map failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return ret;
    }

    return CCEPH_OK;
}

int cceph_mem_store_do_op_obj_write(
        cceph_mem_store*   os,
        cceph_os_tran_op*  op,
        int64_t            log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);
    assert(log_id, op->cid >= 0);
    assert(log_id, op->oid != NULL);
    assert(log_id, op->offset >= 0);
    assert(log_id, op->length >= 0);
    if (op->length > 0) {
        //Touch may not have a data
        assert(log_id, op->data != NULL);
    }

    LOG(LL_INFO, log_id, "Execute write op, cid %d, oid %s, offset %ld, length %ld, log_id %ld.",
            op->cid, op->oid, op->offset, op->length, op->log_id);

    log_id = op->log_id; //We will use op's log_id from here

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&os->colls, op->cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute write op failed, search cid %d failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_map_search(&cnode->objects, op->oid, &onode, log_id);
    if (ret == CCEPH_ERR_MAP_NODE_NOT_EXIST) {
        //if onode don't existed, create it
        ret = cceph_mem_store_object_node_new(op->oid, &onode, log_id);
        if (ret != CCEPH_OK) {
            LOG(LL_ERROR, log_id, "Execute Write op failed, create object node failed, errno %d(%s).",
                    ret, cceph_errno_str(ret));
            return ret;
        }

        ret = cceph_mem_store_object_node_map_insert(&cnode->objects, onode, log_id);
        if (ret != CCEPH_OK) {
            cceph_mem_store_object_node_free(&onode, log_id);
            LOG(LL_ERROR, log_id, "Execute Write op failed, insert into coll failed, errno %d(%s).",
                    ret, cceph_errno_str(ret));
            return ret;
        }
    } else if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute write op failed, search oid %s failed, errno %d(%s).",
                op->oid, ret, cceph_errno_str(ret));
        return ret;
    }

    //Is a touch operation?
    if (op->length == 0) {
        return CCEPH_OK;
    }

    assert(log_id, strcmp(op->oid, onode->oid) == 0);
    if (onode->length < op->offset + op->length) {
        //We don't have enough space, we should expend the data
        int64_t new_length = op->offset + op->length;
        char*   new_data   = (char*)malloc(sizeof(char) * new_length);
        if (new_data == NULL) {
            return CCEPH_ERR_NO_ENOUGH_MEM;
        }

        //cp old data to new data;
        memset(new_data, 0, new_length);
        memcpy(new_data, onode->data, onode->length);

        char* old_data = onode->data;
        onode->data    = new_data;
        onode->length  = new_length;

        free(old_data);
    }

    //Write op
    memcpy(onode->data + op->offset, op->data, op->length);

    return CCEPH_OK;
}

int cceph_mem_store_do_op_obj_remove(
        cceph_mem_store*  os,
        cceph_os_tran_op* op,
        int64_t           log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);
    assert(log_id, op->cid >= 0);
    assert(log_id, op->oid != NULL);

    LOG(LL_INFO, log_id, "Execute remove op, cid %d, oid %s, log_id %ld.",
            op->cid, op->oid, op->log_id);

    log_id = op->log_id; //We will use op's log_id from here

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&os->colls, op->cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute remove op failed, search cid %d failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_map_search(&cnode->objects, op->oid, &onode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute remove op failed, search oid %d failed, errno %d(%s).",
                op->oid, ret, cceph_errno_str(ret));
        return CCEPH_ERR_OBJECT_NOT_EXIST;
    }

    cceph_mem_store_object_node_map_remove(&cnode->objects, onode, log_id);
    cceph_mem_store_object_node_free(&onode, log_id);

    return CCEPH_OK;
}
int cceph_mem_store_do_op_obj_map(
        cceph_mem_store*   os,
        cceph_os_tran_op*  op,
        int64_t            log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);
    assert(log_id, op->cid >= 0);
    assert(log_id, op->oid != NULL);
    assert(log_id, op->map != NULL);

    LOG(LL_INFO, log_id, "Execute ObjectMap op, cid %d, oid %s, log_id %ld.", op->cid, op->oid, op->log_id);

    log_id = op->log_id; //We will use op's log_id from here

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&os->colls, op->cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ObjectMap op failed, cid %d not existed.", op->cid);
        return ret;
    }

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_map_search(&cnode->objects, op->oid, &onode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ObjectMap failed, search oid %d failed, errno %d(%s).",
                op->oid, ret, cceph_errno_str(ret));
        return ret;
    }

    ret = cceph_os_map_update(&onode->map, op->map, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ObjectMap op failed, update object map failed, errno %d(%s).",
                op->cid, ret, cceph_errno_str(ret));
        return ret;
    }

    return CCEPH_OK;
}

int cceph_mem_store_do_op(
        cceph_mem_store*  os,
        cceph_os_tran_op* op,
        int64_t           log_id) {

    assert(log_id, os != NULL);
    assert(log_id, op != NULL);

    int ret = CCEPH_OK;
    switch(op->op) {
        case CCEPH_OS_OP_NOOP:
            ret = CCEPH_OK;
            break;
        case CCEPH_OS_OP_COLL_CREATE:
            ret = cceph_mem_store_do_op_coll_create(os, op, log_id);
            break;
        case CCEPH_OS_OP_COLL_REMOVE:
            ret = cceph_mem_store_do_op_coll_remove(os, op, log_id);
            break;
        case CCEPH_OS_OP_COLL_MAP:
            ret = cceph_mem_store_do_op_coll_map(os, op, log_id);
            break;
        case CCEPH_OS_OP_OBJ_TOUCH:
            op->length = 0; //this means touch
            ret = cceph_mem_store_do_op_obj_write(os, op, log_id);
            break;
        case CCEPH_OS_OP_OBJ_WRITE:
            ret = cceph_mem_store_do_op_obj_write(os, op, log_id);
            break;
        case CCEPH_OS_OP_OBJ_REMOVE:
            ret = cceph_mem_store_do_op_obj_remove(os, op, log_id);
            break;
        case CCEPH_OS_OP_OBJ_MAP:
            ret = cceph_mem_store_do_op_obj_map(os, op, log_id);
            break;
        default:
            ret = CCEPH_ERR_UNKNOWN_OS_OP;
    }

    if (ret == CCEPH_OK) {
        LOG(LL_INFO, log_id, "do_op %s(%d) success.",
                cceph_os_op_to_str(op->op), op->op);
    } else {
        LOG(LL_ERROR, log_id, "do_op %s(%d) failed, errno %d(%s).",
                cceph_os_op_to_str(op->op), op->op,
                ret, cceph_errno_str(ret));
    }

    return ret;
}

int cceph_mem_store_submit_tran(
        cceph_object_store*   os,
        cceph_os_tran*        tran,
        int64_t               log_id) {

    assert(log_id, os != NULL);
    assert(log_id, tran != NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;
    int op_count = cceph_os_tran_get_op_count(tran, log_id);

    LOG(LL_INFO, log_id, "Submit transaction with %d ops.", op_count);
    pthread_mutex_lock(&mem_store->lock);

    int ret = CCEPH_OK;
    int i = 0;
    int success_count = 0;
    for (i = 0; i < op_count; i++) {
        cceph_os_tran_op* op = cceph_os_tran_get_op(tran, i, log_id);
        ret = cceph_mem_store_do_op(mem_store, op, log_id);
        if (ret != CCEPH_OK) {
            break;
        } else {
            success_count++;
        }
    }

    pthread_mutex_unlock(&mem_store->lock);
    LOG(LL_INFO, log_id, "Transaction executed with %d/%d ops done.", success_count, op_count);
    return ret;
}

int cceph_mem_store_read_obj(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        int64_t             offset,
        int64_t             length,
        int64_t*            result_length,
        char**              result_data,
        int64_t             log_id) {

    assert(log_id, os              != NULL);
    assert(log_id, oid             != NULL);
    assert(log_id, offset          >= 0);
    assert(log_id, result_data     != NULL);
    assert(log_id, *result_data    == NULL);
    assert(log_id, result_length   != NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;

    pthread_mutex_lock(&mem_store->lock);

    LOG(LL_INFO, log_id, "Execute read op, cid %d, oid %s, offset %ld, length %ld, log_id %ld.",
            cid, oid, offset, length, log_id);

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&mem_store->colls, cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute read op failed, search cid %d failed, errno %d(%s).",
                cid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_map_search(&cnode->objects, oid, &onode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute read op failed, search oid %s failed, errno %d(%s).",
                oid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_OBJECT_NOT_EXIST;
    }

    if (onode->length == 0 || onode->length <= offset) {
        *result_length = 0;
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_OK;
    }

    int read_length = length;
    if (read_length <= 0) {
        read_length = onode->length;
    }
    if (read_length > onode->length - offset) {
        read_length = onode->length - offset;
    }

    *result_data = malloc(sizeof(char) * read_length);
    if (*result_data == NULL) {
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    memcpy(*result_data, onode->data + offset, read_length);

    *result_length = read_length;

    pthread_mutex_unlock(&mem_store->lock);

    return CCEPH_OK;
}

extern int cceph_mem_store_list_coll(
        cceph_object_store*  os,
        int32_t*             coll_id_list_length,
        cceph_os_coll_id_t** coll_id_list,
        int64_t              log_id) {

    assert(log_id, os != NULL);
    assert(log_id, coll_id_list_length != NULL);
    assert(log_id, coll_id_list != NULL);
    assert(log_id, *coll_id_list == NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;
    pthread_mutex_lock(&mem_store->lock);

    LOG(LL_INFO, log_id, "Execute ListCollection op, log_id %ld.", log_id);

    int ret = CCEPH_OK;
    int coll_count = 0;
    cceph_rb_node* coll_rb_node = cceph_rb_first(&mem_store->colls);
    while (coll_rb_node) {
        coll_count++;
        coll_rb_node = cceph_rb_next(coll_rb_node);
    }

    *coll_id_list = (cceph_os_coll_id_t*)malloc(sizeof(cceph_os_coll_id_t) * coll_count);
    if (*coll_id_list == NULL) {
        LOG(LL_ERROR, log_id, "Execute ListCollection failed, No enough memory");
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    *coll_id_list_length = coll_count;

    coll_rb_node = cceph_rb_first(&mem_store->colls);
    cceph_os_coll_id_t* coll_id_list_ptr = *coll_id_list;
    while (coll_rb_node) {
        cceph_mem_store_coll_node *cnode = cceph_container_of(coll_rb_node, cceph_mem_store_coll_node, node);

        *coll_id_list_ptr = cnode->cid;
        coll_id_list_ptr++;

        coll_rb_node = cceph_rb_next(coll_rb_node);
    }

    pthread_mutex_unlock(&mem_store->lock);
    return ret;
}

extern int cceph_mem_store_exist_coll(
        cceph_object_store*  os,
        cceph_os_coll_id_t   cid,
        int8_t*              is_existed,
        int64_t              log_id) {

    assert(log_id, os != NULL);
    assert(log_id, is_existed != NULL);

    LOG(LL_INFO, log_id, "Execute ExistCollection op, log_id %ld.", log_id);

    cceph_mem_store*           mem_store = (cceph_mem_store*)os;
    cceph_mem_store_coll_node* cnode     = NULL;

    pthread_mutex_lock(&mem_store->lock);
    cceph_mem_store_coll_node_map_search(&mem_store->colls, cid, &cnode, log_id);
    pthread_mutex_unlock(&mem_store->lock);

    *is_existed = cnode == NULL ? 0 : 1;
    return CCEPH_OK;
}

extern int cceph_mem_store_read_coll_map(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        cceph_rb_root*      map,
        int64_t             log_id) {
    assert(log_id, os  != NULL);
    assert(log_id, map != NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;
    pthread_mutex_lock(&mem_store->lock);

    LOG(LL_INFO, log_id, "Execute ReadCollectionMap op, cid %d, log_id %ld.",
            cid, log_id);

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&mem_store->colls, cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadCollectionMap failed, search cid %d failed, errno %d(%s).",
                cid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    ret = cceph_os_map_update(map, &cnode->map, log_id);
    assert(log_id, ret == CCEPH_OK);

    pthread_mutex_unlock(&mem_store->lock);
    return ret;
}
int cceph_mem_store_read_coll_map_key(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         key,
        int32_t*            result_value_length,
        char**              result_value,
        int64_t             log_id) {

    assert(log_id, os  != NULL);
    assert(log_id, key != NULL);
    assert(log_id, result_value_length != NULL);
    assert(log_id, result_value != NULL);
    assert(log_id, *result_value == NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;
    pthread_mutex_lock(&mem_store->lock);

    LOG(LL_INFO, log_id, "Execute ReadCollectionMapKey op, cid %d, log_id %ld.",
            cid, log_id);

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&mem_store->colls, cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadCollectionMapKey failed, search cid %d failed, errno %d(%s).",
                cid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    cceph_os_map_node* map_node = NULL;
    ret = cceph_os_map_node_map_search(&cnode->map, key, &map_node, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadCollectionMapKey failed, key %s not found, errno %d(%s).",
                key, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return ret;
    }

    int32_t value_length = map_node->value_length;
    *result_value = (char*)malloc(sizeof(char) * value_length);
    if (*result_value == NULL) {
        LOG(LL_ERROR, log_id, "Execute ReadCollectionMapKey failed, no enough memory, errno %d(%s).",
                ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    *result_value_length = value_length;
    memcpy(*result_value, map_node->value, value_length);

    pthread_mutex_unlock(&mem_store->lock);
    return ret;
}

extern int cceph_mem_store_read_obj_map(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        cceph_rb_root*      map,
        int64_t             log_id) {

    assert(log_id, os  != NULL);
    assert(log_id, oid != NULL);
    assert(log_id, map != NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;
    pthread_mutex_lock(&mem_store->lock);

    LOG(LL_INFO, log_id, "Execute ReadObjectMap op, cid %d, oid %s, log_id %ld.",
            cid, oid, log_id);

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&mem_store->colls, cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadObjectMap failed, search cid %d failed, errno %d(%s).",
                cid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return ret;
    }

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_map_search(&cnode->objects, oid, &onode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadObjectMap failed, search oid %s failed, errno %d(%s).",
                oid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return ret;
    }

    ret = cceph_os_map_update(map, &onode->map, log_id);
    assert(log_id, ret == CCEPH_OK);

    pthread_mutex_unlock(&mem_store->lock);
    return ret;
}
int cceph_mem_store_read_obj_map_key(
        cceph_object_store* os,
        cceph_os_coll_id_t  cid,
        const char*         oid,
        const char*         key,
        int32_t*            result_value_length,
        char**              result_value,
        int64_t             log_id) {

    assert(log_id, os  != NULL);
    assert(log_id, oid != NULL);
    assert(log_id, key != NULL);
    assert(log_id, result_value_length != NULL);
    assert(log_id, result_value != NULL);
    assert(log_id, *result_value == NULL);

    cceph_mem_store* mem_store = (cceph_mem_store*)os;
    pthread_mutex_lock(&mem_store->lock);

    LOG(LL_INFO, log_id, "Execute ReadObjectMapKey op, cid %d, oid %s, log_id %ld.",
            cid, oid, log_id);

    cceph_mem_store_coll_node *cnode = NULL;
    int ret = cceph_mem_store_coll_node_map_search(&mem_store->colls, cid, &cnode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadObjectMapKey failed, search cid %d failed, errno %d(%s).",
                cid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_COLL_NOT_EXIST;
    }

    cceph_mem_store_object_node *onode = NULL;
    ret = cceph_mem_store_object_node_map_search(&cnode->objects, oid, &onode, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadObjectMapKey failed, search oid %s failed, errno %d(%s).",
                oid, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return ret;
    }

    cceph_os_map_node* map_node = NULL;
    ret = cceph_os_map_node_map_search(&onode->map, key, &map_node, log_id);
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Execute ReadObjectMapKey failed, key %s not found, errno %d(%s).",
                key, ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return ret;
    }

    int32_t value_length = map_node->value_length;
    *result_value = (char*)malloc(sizeof(char) * value_length);
    if (*result_value == NULL) {
        LOG(LL_ERROR, log_id, "Execute ReadObjectMapKey failed, no enough memory, errno %d(%s).",
                ret, cceph_errno_str(ret));
        pthread_mutex_unlock(&mem_store->lock);
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    *result_value_length = value_length;
    memcpy(*result_value, map_node->value, value_length);

    pthread_mutex_unlock(&mem_store->lock);
    return ret;
}

