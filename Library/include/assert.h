#ifndef __ASSERT_H
#define __ASSERT_H
#ifndef __TYPES_H
#include <types.h>
#endif

/* ANSI compilers only version ! */

/* If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

#define assert(expr)		((void) 0)

#else /* NDEBUG */

extern void __assert __P((char *, char *, int));

#define assert(expr)	\
  ((void) ((expr) || (__assert (__STRING(expr),  __FILE__, __LINE__), 0)))

#endif /* NDEBUG */

extern void __errput __P((char *));

#endif /* __ASSERT_H */

