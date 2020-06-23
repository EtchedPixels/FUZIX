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

/* 630x wins on this one! */
#define	ntohs(x)	(x)
#define ntohl(x)	(x)

/* 6303 doesn't benefit from making a few key variables in
   non-reentrant functions static - FIXME, evaluate this carefully esp
   wrt zero page  */
#define staticfast	auto

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef struct {
   uint32_t high;	   /* FIXME: check this matches long long */
   uint32_t low;
} time_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t high;
      uint16_t low;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;


#define cpu_to_le16(x)	swab(x)
#define le16_to_cpu(x)	swab(x)

/* This is horrid, asm it */
#define cpu_to_le32(x)	((((uint32_t)cpu_to_le16((x) & 0xFFFF)) << 16) | \
				(uint32_t)cpu_to_le16((x) >> 16))
#define le32_to_cpu(x)	cpu_to_le32(x)

/* Sane behaviour for unused parameters */
#define used(x)

#define BIG_ENDIAN

#define __packed
#define barrier()

#define __fastcall
#define inline
