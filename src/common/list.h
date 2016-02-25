#ifndef CCEPH_COMMON_LIST_
#define CCEPH_COMMON_LIST_

#define CCEPH_LIST_POISON_POINTER_DELTA 0

#define CCEPH_LIST_POISON1  ((char *) 0x00100100 + CCEPH_LIST_POISON_POINTER_DELTA)
#define CCEPH_LIST_POISON2  ((char *) 0x00200200 + CCEPH_LIST_POISON_POINTER_DELTA)

#define cceph_list_offsetof(type, member)  (size_t)(&((type*)0)->member)

#define cceph_list_container_of(ptr, type, member) ({              \
        const typeof(((type *)0)->member)*__mptr = (ptr);          \
    (type *)((char *)__mptr - cceph_list_offsetof(type, member)); })

struct cceph_list_head
{
    struct cceph_list_head *prev;
    struct cceph_list_head *next;
};

static inline void cceph_list_head_init(struct cceph_list_head *list)
{
    list->prev = list;
    list->next = list;
};

static inline void __cceph_list_add(struct cceph_list_head *new_node,
    struct cceph_list_head *prev, struct cceph_list_head *next)
{
    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = next;
    next->prev = new_node;
};

static inline void cceph_list_add(struct cceph_list_head *new_node , struct cceph_list_head *head)
{
    __cceph_list_add(new_node, head, head->next);
};
static inline void cceph_list_add_tail(struct cceph_list_head *new_node, struct cceph_list_head *head)
{
    __cceph_list_add(new_node, head->prev, head);
};

static inline  void __cceph_list_delete(struct cceph_list_head *prev, struct cceph_list_head *next)
{
    prev->next = next;
    next->prev = prev;
};

static inline void cceph_list_delete(struct cceph_list_head *entry)
{
    __cceph_list_delete(entry->prev, entry->next);
    entry->next = (struct cceph_list_head*)CCEPH_LIST_POISON1;
    entry->prev = (struct cceph_list_head*)CCEPH_LIST_POISON2;
};

static inline void cceph_list_move(struct cceph_list_head *list, struct cceph_list_head *head)
{
        __cceph_list_delete(list->prev, list->next);
        cceph_list_add(list, head);
};

static inline void cceph_list_move_tail(struct cceph_list_head *list,
                      struct cceph_list_head *head)
{
        __cceph_list_delete(list->prev, list->next);
        cceph_list_add_tail(list, head);
};
#define cceph_list_entry(ptr, type, member) \
    cceph_list_container_of(ptr, type, member)

#define cceph_list_first_entry(ptr, type, member) \
    cceph_list_entry((ptr)->next, type, member)

#define cceph_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#endif
