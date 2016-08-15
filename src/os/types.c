#include "os/types.h"

#include <string.h>

#include "common/assert.h"
#include "common/errno.h"
#include "common/log.h"
#include "common/rbtree.h"

extern int cceph_os_coll_id_cmp(cceph_os_coll_id_t cid_a, cceph_os_coll_id_t cid_b) {
    if (cid_a > cid_b) {
        return 1;
    } else if (cid_a < cid_b) {
        return -1;
    } else {
        return 0;
    }
}

const char* cceph_os_op_to_str(int op) {
    switch(op) {
        case CCEPH_OS_OP_NOOP:
            return "NoOp";
        case CCEPH_OS_OP_COLL_CREATE:
            return "CollectionCreate";
        case CCEPH_OS_OP_COLL_REMOVE:
            return "CollectionRemove";
        case CCEPH_OS_OP_COLL_MAP:
            return "CollectionMap";
        case CCEPH_OS_OP_OBJ_TOUCH:
            return "ObjectTouch";
        case CCEPH_OS_OP_OBJ_WRITE:
            return "ObjectWrite";
        case CCEPH_OS_OP_OBJ_MAP:
            return "ObjectMap";
        case CCEPH_OS_OP_OBJ_REMOVE:
            return "ObjectRemove";
        default:
            return "UnknownOp";
    }

}
int cceph_os_map_node_new(
        const char*         key,
        const char*         value,
        int                 value_length,
        cceph_os_map_node** node,
        int64_t             log_id) {

    assert(log_id, node  != NULL);
    assert(log_id, key   != NULL);
    if (value_length > 0) {
        assert(log_id, value != NULL);
    }

    *node = (cceph_os_map_node*)malloc(sizeof(cceph_os_map_node));
    if (*node == NULL) {
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }

    int key_length = strlen(key) + 1;
    (*node)->key = (char*)malloc(sizeof(char) * key_length);
    if ((*node)->key == NULL) {
        free(*node);
        *node = NULL;
        return CCEPH_ERR_NO_ENOUGH_MEM;
    }
    memset((*node)->key, 0, key_length);
    strcpy((*node)->key, key);

    if (value_length > 0) {
        (*node)->value = (char*)malloc(sizeof(char) * value_length);
        if ((*node)->value == NULL) {
            free((*node)->key);
            free(*node);
            *node = NULL;
            return CCEPH_ERR_NO_ENOUGH_MEM;
        }
        memcpy((*node)->value, value, value_length);
        (*node)->value_length = value_length;
    } else {
        (*node)->value = NULL;
        (*node)->value_length = 0;
    }

    return CCEPH_OK;
}

int cceph_os_map_node_free(
        cceph_os_map_node** node,
        int64_t             log_id) {

    assert(log_id, node != NULL);
    assert(log_id, *node != NULL);

    cceph_os_map_node* map_node = *node;
    if (map_node->key != NULL) {
        free(map_node->key);
        map_node->key = NULL;
    }
    if (map_node->value != NULL) {
        free(map_node->value);
        map_node->value = NULL;
    }

    free(map_node);
    *node = NULL;

    return CCEPH_OK;
}

CCEPH_IMPL_MAP(os_map_node,  const char*, key, strcmp);

int cceph_os_map_update(cceph_rb_root* result_tree, cceph_rb_root* input_tree, int64_t log_id) {
    assert(log_id, result_tree != NULL);
    assert(log_id, input_tree  != NULL);

    int ret = CCEPH_OK;
    cceph_rb_node* input_rb_node = cceph_rb_first(input_tree);
    while (input_rb_node) {
        cceph_os_map_node *input_node = cceph_container_of(input_rb_node, cceph_os_map_node, node);

        const char* key          = input_node->key;
        const char* value        = input_node->value;
        int32_t     value_length = input_node->value_length;

        assert(log_id, key != NULL);
        if (value_length > 0) {
            assert(log_id, value != NULL);
        }

        cceph_os_map_node* result_node = NULL;
        ret = cceph_os_map_node_map_search(result_tree, key, &result_node, log_id);
        if (result_node == NULL) {
            if (input_node->value_length > 0) {
                //Add new node to result_tree
                ret = cceph_os_map_node_new(key, value, value_length, &result_node, log_id);
                if (ret != CCEPH_OK) {
                    goto label_ret;
                }
                ret = cceph_os_map_node_map_insert(result_tree, result_node, log_id);
                if (ret != CCEPH_OK) {
                    goto label_ret;
                }
            }
        } else {
            //Remove the old first
            ret = cceph_os_map_node_map_remove(result_tree, result_node, log_id);
            if (ret != CCEPH_OK) {
                goto label_ret;
            }
            ret = cceph_os_map_node_free(&result_node, log_id);
            if (ret != CCEPH_OK) {
                goto label_ret;
            }

            //Remove + Add = Update
            if (input_node->value_length > 0) {
                ret = cceph_os_map_node_new(key, value, value_length, &result_node, log_id);
                if (ret != CCEPH_OK) {
                    goto label_ret;
                }

                ret = cceph_os_map_node_map_insert(result_tree, result_node, log_id);
                if (ret != CCEPH_OK) {
                    goto label_ret;
                }
            }
        }

        input_rb_node = cceph_rb_next(input_rb_node);
    }

label_ret:
    if (ret != CCEPH_OK) {
        LOG(LL_ERROR, log_id, "Error in cceph_os_map_update, errno %d(%s)",
                ret, cceph_errno_str(ret));
    }
    return CCEPH_OK;
}

