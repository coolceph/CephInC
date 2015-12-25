#ifndef CCEPH_COMMON_LIST_
#define CCEPH_COMMON_LIST_

#define POISON_POINTER_DELTA 0

#define LIST_POISON1  ((void *) 0x00100100 + POISON_POINTER_DELTA)
#define LIST_POISON2  ((void *) 0x00200200 + POISON_POINTER_DELTA)

#define offsetof(type, member)  (size_t)(&((type*)0)->member)

#define container_of(ptr, type, member) ({          \
        const typeof(((type *)0)->member)*__mptr = (ptr);    \
    (type *)((char *)__mptr - offsetof(type, member)); })

struct list_head
{
    struct list_head *prev;
    struct list_head *next;
};

static inline void init_list_head(struct list_head *list)
{
    list->prev = list;
    list->next = list;
}

static inline void __list_add(struct list_head *new,
    struct list_head *prev, struct list_head *next)
{
    prev->next = new;
    new->prev = prev;
    new->next = next;
    next->prev = new;
}

static inline void list_add(struct list_head *new , struct list_head *head)
{
    __list_add(new, head, head->next);
}
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

static inline  void __list_del(struct list_head *prev, struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
                      struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add_tail(list, head);
}
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#endif
