#ifndef CCEPH_ASSERT_H
#define CCEPH_ASSERT_H

extern void __cceph_assert_fail(const char *assertion, const char *file, int line, const char *function)
  __attribute__ ((__noreturn__));
extern void __cceph_assertf_fail(const char *assertion, const char *file, int line, const char *function, const char* msg, ...)
  __attribute__ ((__noreturn__));
extern void __cceph_assert_warn(const char *assertion, const char *file, int line, const char *function);

#ifdef HAVE_STATIC_CAST
# define __CCEPH_ASSERT_VOID_CAST static_cast<void>
#else
# define __CCEPH_ASSERT_VOID_CAST (void)
#endif


#define ceph_assert(expr)							\
  ((expr)								\
   ? __CCEPH_ASSERT_VOID_CAST (0)					\
   : __cceph_assert_fail (__STRING(expr), __FILE__, __LINE__, __CCEPH_ASSERT_FUNCTION))

#define assert_warn(expr)							\
  ((expr)								\
   ? __CCEPH_ASSERT_VOID_CAST (0)					\
   : __cceph_assert_warn (__STRING(expr), __FILE__, __LINE__, __CCEPH_ASSERT_FUNCTION))

#define ceph_abort() assert(0)

// wipe any prior assert definition
#ifdef assert
# undef assert
#endif

// make _ASSERT_H something that *must* have a value other than what
// /usr/include/assert.h gives it (nothing!), so that we detect when
// our assert is clobbered.
#undef _ASSERT_H
#define _ASSERT_H "CCEPH_ASSERT"

// make __ASSERT_FUNCTION empty (/usr/include/assert.h makes it a function)
// and make our encoding macros break if it non-empty.
#undef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION

#define assert(expr)							\
  ((expr)								\
   ? __CCEPH_ASSERT_VOID_CAST (0)					\
   : __cceph_assert_fail (__STRING(expr), __FILE__, __LINE__, __func__))

#endif
