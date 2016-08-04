/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  linux/include/linux/rbtree.h

  To use rbtrees you'll have to implement your own insert and search cores.
  This will avoid us to use callbacks and to drop drammatically performances.
  I know it's not the cleaner way,  but in C (not in C++) to get
  performances and genericity...

  Some example of insert and search follows here. The search is a plain
  normal search over an ordered tree. The insert instead must be implemented
  int two steps: as first thing the code must insert the element in
  order as a red leaf in the tree, then the support library function
  rb_insert_color() must be called. Such function will do the
  not trivial work to rebalance the rbtree if necessary.

-----------------------------------------------------------------------
static inline struct page * rb_search_page_cache(struct inode * inode,
						 unsigned long offset)
{
	cceph_rb_node * n = inode->i_rb_page_cache.rb_node;
	struct page * page;

	while (n)
	{
		page = rb_entry(n, struct page, rb_page_cache);

		if (offset < page->offset)
			n = n->rb_left;
		else if (offset > page->offset)
			n = n->rb_right;
		else
			return page;
	}
	return NULL;
}

static inline struct page * __rb_insert_page_cache(struct inode * inode,
						   unsigned long offset,
						   cceph_rb_node * node)
{
	cceph_rb_node ** p = &inode->i_rb_page_cache.rb_node;
	cceph_rb_node * parent = NULL;
	struct page * page;

	while (*p)
	{
		parent = *p;
		page = rb_entry(parent, struct page, rb_page_cache);

		if (offset < page->offset)
			p = &(*p)->rb_left;
		else if (offset > page->offset)
			p = &(*p)->rb_right;
		else
			return page;
	}

	rb_link_node(node, parent, p);

	return NULL;
}

static inline struct page * rb_insert_page_cache(struct inode * inode,
						 unsigned long offset,
						 cceph_rb_node * node)
{
	struct page * ret;
	if ((ret = __rb_insert_page_cache(inode, offset, node)))
		goto out;
	rb_insert_color(node, &inode->i_rb_page_cache);
 out:
	return ret;
}
-----------------------------------------------------------------------
*/

#include <stdlib.h>

#ifndef	_CCEPH_RBTREE_H
#define	_CCEPH_RBTREE_H

#define	CCEPH_RB_RED    0
#define	CCEPH_RB_BLACK  1

typedef struct cceph_rb_node_ cceph_rb_node_;
struct cceph_rb_node_ {
	unsigned long  rb_parent_color;
	struct cceph_rb_node_ *rb_right;
	struct cceph_rb_node_ *rb_left;
} __attribute__((aligned(sizeof(long))));
typedef struct cceph_rb_node_ cceph_rb_node;

typedef struct {
	cceph_rb_node *rb_node;
} cceph_rb_root;

#define cceph_offsetof(type, member)  (size_t)(&((type*)0)->member)

#define cceph_container_of(ptr, type, member) ({                         \
        const typeof(((type *)0)->member)*__mptr = (ptr);                \
        (type *)((char *)__mptr - cceph_offsetof(type, member)); })

#define cceph_rb_parent(r)     ((cceph_rb_node *)((r)->rb_parent_color & ~3))
#define cceph_rb_color(r)      ((r)->rb_parent_color & 1)
#define cceph_rb_is_red(r)     (!cceph_rb_color(r))
#define cceph_rb_is_black(r)   cceph_rb_color(r)
#define cceph_rb_set_red(r)    do { (r)->rb_parent_color &= ~1; } while (0)
#define cceph_rb_set_black(r)  do { (r)->rb_parent_color |= 1; } while (0)

#define	cceph_rb_entry(ptr, type, member) cceph_container_of(ptr, type, member)

static inline void cceph_rb_set_parent(cceph_rb_node *rb, cceph_rb_node *p) {
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (unsigned long)p;
}
static inline void cceph_rb_set_color(cceph_rb_node *rb, int color) {
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

#define CCEPH_RB_ROOT               (cceph_rb_root) { NULL, }
#define CCEPH_RB_EMPTY_ROOT(root)   ((root)->rb_node == NULL)
#define CCEPH_RB_EMPTY_NODE(node)   (cceph_rb_parent(node) == node)
#define CCEPH_RB_CLEAR_NODE(node)   (cceph_rb_set_parent(node, node))

extern void cceph_rb_insert_color(cceph_rb_node *, cceph_rb_root *);
extern void cceph_rb_erase(cceph_rb_node *, cceph_rb_root *);

/* Find logical next and previous nodes in a tree */
extern cceph_rb_node *cceph_rb_next(const cceph_rb_node *);
extern cceph_rb_node *cceph_rb_prev(const cceph_rb_node *);
extern cceph_rb_node *cceph_rb_first(const cceph_rb_root *);
extern cceph_rb_node *cceph_rb_last(const cceph_rb_root *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
extern void cceph_rb_replace_node(
        cceph_rb_node *victim_node,
        cceph_rb_node *new_node,
        cceph_rb_root *root);

static inline void cceph_rb_link_node(
        cceph_rb_node *node,
        cceph_rb_node *parent,
        cceph_rb_node **rb_link) {

	node->rb_parent_color = (unsigned long )parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#define CCEPH_DEFINE_MAP(type, key_type, key_name) \
extern int cceph_##type##_map_insert(              \
        cceph_rb_root *root,                       \
        cceph_##type  *node,                       \
        int64_t       log_id);                     \
extern int cceph_##type##_map_remove(              \
        cceph_rb_root *root,                       \
        cceph_##type  *node,                       \
        int64_t       log_id);                     \
extern int cceph_##type##_map_search(              \
        cceph_rb_root *root,                       \
        key_type      key_name,                    \
        cceph_##type  **result,                    \
        int64_t       log_id);                     \

#define CCEPH_IMPL_MAP(type, key_type, key_name, cmp_method)                    \
int cceph_##type##_map_insert(                                                  \
        cceph_rb_root *root,                                                    \
        cceph_##type  *node,                                                    \
        int64_t       log_id) {                                                 \
                                                                                \
    assert(log_id, root != NULL);                                               \
    assert(log_id, node != NULL);                                               \
                                                                                \
    cceph_rb_node **new = &(root->rb_node), *parent = NULL;                     \
                                                                                \
    /* Figure out where to put new node */                                      \
    while (*new) {                                                              \
        cceph_##type *this = cceph_container_of(*new, cceph_##type, node);      \
        int result = cmp_method(node->key_name, this->key_name);                \
                                                                                \
        parent = *new;                                                          \
        if (result < 0) {                                                       \
            new = &((*new)->rb_left);                                           \
        } else if (result > 0) {                                                \
            new = &((*new)->rb_right);                                          \
        } else {                                                                \
            return CCEPH_ERR_MAP_NODE_ALREADY_EXIST;                            \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Add new node and rebalance tree. */                                      \
    cceph_rb_link_node(&node->node, parent, new);                               \
    cceph_rb_insert_color(&node->node, root);                                   \
                                                                                \
    return CCEPH_OK;                                                            \
}                                                                               \
                                                                                \
int cceph_##type##_map_remove(                                                  \
        cceph_rb_root *root,                                                    \
        cceph_##type  *node,                                                    \
        int64_t       log_id) {                                                 \
                                                                                \
    assert(log_id, root != NULL);                                               \
    assert(log_id, node != NULL);                                               \
                                                                                \
    cceph_rb_erase(&node->node, root);                                          \
                                                                                \
    return CCEPH_OK;                                                            \
}                                                                               \
                                                                                \
int cceph_##type##_map_search(                                                  \
        cceph_rb_root *root,                                                    \
        key_type      key,                                                      \
        cceph_##type  **result,                                                 \
        int64_t       log_id) {                                                 \
                                                                                \
    assert(log_id, root != NULL);                                               \
    assert(log_id, result != NULL);                                             \
    assert(log_id, *result == NULL);                                            \
                                                                                \
    cceph_rb_node *node = root->rb_node;                                        \
                                                                                \
    while (node) {                                                              \
        cceph_##type *data = cceph_container_of(node, cceph_##type, node);      \
                                                                                \
        int ret = cmp_method(key, data->key_name);                              \
        if (ret < 0) {                                                          \
            node = node->rb_left;                                               \
        } else if (ret > 0) {                                                   \
            node = node->rb_right;                                              \
        } else {                                                                \
            *result = data;                                                     \
            return CCEPH_OK;                                                    \
        }                                                                       \
    }                                                                           \
    return CCEPH_ERR_MAP_NODE_NOT_EXIST;                                        \
}

#endif
