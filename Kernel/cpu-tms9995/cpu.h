#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)

extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
extern int strcmp(const char *, const char *);
extern size_t strlen(const char *);
extern uint16_t swab(uint16_t);

#define	ntohs(x)	(x)
#define ntohl(x)	(x)

/* TMS99xx doesn't benefit from making a few key variables in
   non-reentrant functions static */
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


#define cpu_to_le16(x)	swab(x)
#define le16_to_cpu(x)	swab(x)
#define cpu_to_le32(x)	((((uint32_t)cpu_to_le16((x) & 0xFFFF)) << 16) | \
				(uint32_t)cpu_to_le16((x) >> 16))
#define le32_to_cpu(x)	cpu_to_le32(x)

/* Sane behaviour for unused parameters */
#define used(x)

#define gcc_miscompile_workaround()	__asm("":::"memory")

#define BIG_ENDIAN

/* Use with care on a TMS99xx - it's a word machine at heart */
#define __packed		__attribute__((packed))
#define barrier()		asm volatile("":::"memory")

#define __fastcall
