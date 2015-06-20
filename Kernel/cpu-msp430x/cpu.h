#include <stddef.h>
#include <stdint.h>

typedef uint8_t irqflags_t;

typedef int16_t arg_t;
typedef uint16_t uarg_t;
typedef uint16_t uaddr_t;
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint32_t clock_t;

extern void ei(void);
extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);

#undef EMAGIC             /* No executable header for MSP430X */

extern void* memcpy(void*, const void*, size_t);
extern void* memset(void*, int, size_t);
extern size_t strlen(const char *);
extern uint16_t swab(uint16_t);

/* MSP430 doesn't benefit from making a few key variables in
   non-reentrant functions static */
#define staticfast auto

/* FIXME: should be 64bits - need to add helpers and struct variants */
typedef struct {
   uint32_t low;
   uint32_t high;
} time_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t low;
      uint16_t high;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;

#define used(x)

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)

/* Bank attributes --- unused */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD

