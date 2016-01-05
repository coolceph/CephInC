#ifndef CCEPH_ATOMIC_H_
#define CCEPH_ATOMIC_H_

#include "include/int_types.h"
#include "include/types.h"

#define LOCK "lock ; "

typedef struct
{
    volatile int32_t counter32;
} atomic_t;

typedef struct
{
    volatile int64_t counter64;
} atomic64_t;

int32_t atomic_get(atomic_t *v);
int64_t atomic_get64(atomic64_t *v);

void atomic_set(atomic_t *v, int32_t i);
void atomic_set64(atomic64_t *v, int64_t i);

int32_t atomic_add(atomic_t *v, int32_t i);
int64_t atomic_add64(atomic64_t *v, int64_t i);

int32_t atomic_sub(atomic_t *v, int32_t i);
int64_t atomic_sub64(atomic64_t *v, int64_t i);

void atomic_inc(atomic_t *v);
void atomic_inc64(atomic64_t *v);

void atomic_dec(atomic_t *v);
void atomic_dec64(atomic64_t *v);

int32_t atomic_exchange(atomic_t *v, int32_t i);
int64_t atomic_exchange64(atomic64_t *v, int64_t i);

#endif
