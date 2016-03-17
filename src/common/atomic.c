
#include "common/atomic.h"

inline int32_t cceph_atomic_get(cceph_atomic_t *v) {
    return v->counter32;
}

inline int64_t cceph_atomic_get64(cceph_atomic64_t *v) {
    return v->counter64;
}

inline void cceph_atomic_set(cceph_atomic_t *v, int32_t i) {
    v->counter32 = i;
}

inline void cceph_atomic_set64(cceph_atomic64_t *v, int64_t i) {
    v->counter64 = i;
}

inline int32_t cceph_atomic_add(cceph_atomic_t *v, int32_t i) {
    int32_t __i;
    __i = i;
    __asm__ __volatile__(
        LOCK "xaddl %0, %1"
        :"+r" (i), "+m" (v->counter32)
        : : "memory");
    return i + __i;
}

inline int64_t cceph_atomic_add64(cceph_atomic64_t *v, int64_t i) {
    int64_t __i;
    __i = i;
    __asm__ __volatile__(
        LOCK "xaddq %0, %1"
        :"+r" (i), "+m" (v->counter64)
        : : "memory");
    return i + __i;
}

inline int32_t cceph_atomic_sub(cceph_atomic_t *v, int32_t i) {
    return cceph_atomic_add(v, -i);
}

inline int64_t cceph_atomic_sub64(cceph_atomic64_t *v, int64_t i) {
    return cceph_atomic_add64(v, -i);
}

inline void cceph_atomic_inc(cceph_atomic_t *v) {
    __asm__ __volatile__(
        LOCK "incl %0"
        :"=m" (v->counter32)
        :"m" (v->counter32));
}

inline void cceph_atomic_inc64(cceph_atomic64_t *v) {
    __asm__ __volatile__(
        LOCK "incq %0"
        :"=m" (v->counter64)
        :"m" (v->counter64));
}

inline void cceph_atomic_dec(cceph_atomic_t *v) {
    __asm__ __volatile__(
        LOCK "decl %0"
        :"=m" (v->counter32)
        :"m" (v->counter32));
}

inline void cceph_atomic_dec64(cceph_atomic64_t *v) {
    __asm__ __volatile__(
        LOCK "decq %0"
        :"=m" (v->counter64)
        :"m" (v->counter64));
}

inline int32_t cceph_atomic_exchange(cceph_atomic_t *v, int32_t i) {
    int32_t result;

    __asm__ __volatile__(
        "xchgl %0, %1"
        :"=r"(result)
        :"m"(v->counter32), "0"(i)
        :"memory");
    return result;
}

inline int64_t cceph_atomic_exchange64(cceph_atomic64_t *v, int64_t i) {
    int64_t result;

    __asm__ __volatile__(
        "xchgq %0, %1"
        :"=r"(result)
        :"m"(v->counter64), "0"(i)
        :"memory");
    return result;
}
