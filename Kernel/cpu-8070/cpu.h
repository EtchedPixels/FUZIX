#include <types.h>

#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)

extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
extern size_t strlen(const char *);
extern uint16_t swab(uint16_t);

#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ntohl(x)	((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | \
                         (((x) & 0xFF0000) >> 8) | (((x >> 24) & 0xFF)))

/* 8070 doesn't benefit from making a few key variables in
   non-reentrant functions static - FIXME, evaluate this carefully esp
   wrt zero page  */
#define staticfast	auto

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t high;
      uint16_t low;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;


#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)

/* Sane behaviour for unused parameters */
#define used(x)

#define HIBYTE32(x)	(((uint8_t *)&(x))[3])

#define __packed
#define barrier()

#define __fastcall
#define inline
