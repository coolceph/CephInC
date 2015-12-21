#ifndef CCEPH_ATOMIC_H_
#define CCEPH_ATOMIC_H_

#define LOCK "lock ; "

typedef struct
{
    volatile int32_t counter32;
} atomic_t;

typedef struct
{
    volatile int64_t counter64;
} atomic64_t;

inline int32_t atomic_get(atomic_t *v)
{
    return v->counter32;
}

inline int64_t atomic_get64(atomic64_t *v)
{
    return v->counter64;
}

inline void atomic_set(atomic_t *v, int32_t i)
{
    v->counter32 = i;
}

inline void atomic_set64(atomic64_t *v, int64_t i)
{
    v->counter64 = i;
}

inline int32_t atomic_add(atomic_t *v, int32_t i)
{
    int32_t __i;
    __i = i;
    __asm__ __volatile__(
        LOCK "xaddl %0, %1"
        :"+r" (i), "+m" (v->counter32)
        : : "memory");
    return i + __i;
}

inline int64_t atomic_add64(atomic64_t *v, int64_t i)
{
    int64_t __i;
    __i = i;
    __asm__ __volatile__(
        LOCK "xaddq %0, %1"
        :"+r" (i), "+m" (v->counter64)
        : : "memory");
    return i + __i;
}

inline int32_t atomic_sub(atomic_t *v, int32_t i)
{
    return atomic_add(v, -i);
}

inline int64_t atomic_sub64(atomic64_t *v, int64_t i)
{
    return atomic_add64(v, -i);
}

inline void atomic_inc(atomic_t *v)
{
    __asm__ __volatile__(
        LOCK "incl %0"
        :"=m" (v->counter32)
        :"m" (v->counter32));
}

inline void atomic_inc64(atomic64_t *v)
{
    __asm__ __volatile__(
        LOCK "incq %0"
        :"=m" (v->counter64)
        :"m" (v->counter64));
}

inline void atomic_dec(atomic_t *v)
{
    __asm__ __volatile__(
        LOCK "decl %0"
        :"=m" (v->counter32)
        :"m" (v->counter32));
}

inline void atomic_dec64(atomic64_t *v)
{
    __asm__ __volatile__(
        LOCK "decq %0"
        :"=m" (v->counter64)
        :"m" (v->counter64));
}

inline int32_t atomic_exchange(atomic_t *v, int32_t i)
{
    int32_t result;

    __asm__ __volatile__(
        "xchgl %0, %1"
        :"=r"(result)
        :"m"(v->counter32), "0"(i)
        :"memory");
    return result;
}

inline int64_t atomic_exchange64(atomic64_t *v, int64_t i)
{
    int64_t result;

    __asm__ __volatile__(
        "xchgq %0, %1"
        :"=r"(result)
        :"m"(v->counter64), "0"(i)
        :"memory");
    return result;
}

#endif
