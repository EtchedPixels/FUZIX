#include <stddef.h>
#include <stdint.h>

/* The MSP430X has 20-bit registers. Yes, really. It defines size_t
 * to be one of those. Use that rather than uint32_t because it's
 * way more efficient. */
typedef uintptr_t uint20_t;
typedef intptr_t int20_t;

typedef uint8_t irqflags_t;

typedef uint32_t arg_t;
typedef uint32_t uarg_t;
typedef uint20_t uaddr_t;
typedef uint20_t usize_t;		/* Largest value passed by userspace */
typedef int20_t susize_t;
typedef uint32_t clock_t;

#define ARGT_IS_BIGGER_THAN_INT 1

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
#define cpu_to_le32(x)  (x)
#define le32_to_cpu(x)  (x)

/* Bank attributes --- unused */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD

