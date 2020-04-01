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
extern uint32_t swab32(uint16_t);
extern uint32_t shuffle32(uint16_t);

/* PDP 11 32bit values are an odd order indeed hence the shuffle32 helper
   we also need below */
#define	ntohs(x)	swab(x)
#define ntohl(x)	swab32(x)

/* PDP-11 doesn't benefit much from making a few key variables in
   non-reentrant functions static */
#define staticfast	auto

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef struct {
   uint32_t low;
   uint32_t high;
} time_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      /* PDP11 endian: be careful here */
      uint16_t high;
      uint16_t low;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;


#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
/* FIXME: make shuffle32 a define */
#define cpu_to_le32(x)  shuffle32(x)
#define le32_to_cpu(x)	cpu_to_le32(x)

/* Sane behaviour for unused parameters */
#define used(x)

#define gcc_miscompile_workaround()

#define BIG_ENDIAN
#define PDP_ENDIAN

#define barrier()		asm volatile("":::"memory")

#define __fastcall
