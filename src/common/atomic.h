#ifndef CCEPH_ATOMIC_H_
#define CCEPH_ATOMIC_H_

#include "common/types.h"

#define LOCK "lock ; "

typedef struct {
    volatile int32_t counter32;
} cceph_atomic_t;

typedef struct {
    volatile int64_t counter64;
} cceph_atomic64_t;

int32_t cceph_atomic_get(cceph_atomic_t *v);
int64_t cceph_atomic_get64(cceph_atomic64_t *v);

void cceph_atomic_set(cceph_atomic_t *v, int32_t i);
void cceph_atomic_set64(cceph_atomic64_t *v, int64_t i);

int32_t cceph_atomic_add(cceph_atomic_t *v, int32_t i);
int64_t cceph_atomic_add64(cceph_atomic64_t *v, int64_t i);

int32_t cceph_atomic_sub(cceph_atomic_t *v, int32_t i);
int64_t cceph_atomic_sub64(cceph_atomic64_t *v, int64_t i);

void cceph_atomic_inc(cceph_atomic_t *v);
void cceph_atomic_inc64(cceph_atomic64_t *v);

void cceph_atomic_dec(cceph_atomic_t *v);
void cceph_atomic_dec64(cceph_atomic64_t *v);

int32_t cceph_atomic_exchange(cceph_atomic_t *v, int32_t i);
int64_t cceph_atomic_exchange64(cceph_atomic64_t *v, int64_t i);

#endif
