#include <stdint.h>
#include <stddef.h>

typedef uint32_t irqflags_t;

typedef int32_t arg_t;
typedef uint32_t uarg_t;		/* Holds arguments */
typedef uint32_t usize_t;		/* Largest value passed by userspace */
typedef int32_t susize_t;
typedef int32_t ssize_t;
typedef uint32_t uaddr_t;
typedef uint32_t uptr_t;		/* User pointer equivalent */

typedef uint32_t clock_t;

#define MAXUSIZE	0xFFFFFFFF

