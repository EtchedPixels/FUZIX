#ifndef __ASSERT_H
#define __ASSERT_H
#ifndef __TYPES_H
#include <types.h>
#endif

/* ANSI compilers only version ! */

/* If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

#define assert(expr)		(0)

#else /* NDEBUG */

extern void __assert(const char *__expr, const char *__file, const int __line);

#define assert(expr)	\
  ((void) ((expr) || (__assert (__STRING(expr),  __FILE__, __LINE__), 0)))

#endif /* NDEBUG */

extern void __errput(const char *__txt);

#endif /* __ASSERT_H */

