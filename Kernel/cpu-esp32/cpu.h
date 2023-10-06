#include <string.h>

/* The LX6 requires aligned accesses. (Annoyingly, it doesn't trap if you get
 * this wrong. It just reads to or writes from the wrong place.) */

#define ALIGNUP(v)   alignup(v, 4)
#define ALIGNDOWN(v) aligndown(v, 4)
#define STACKALIGN(v) aligndown(v, 16)

#define uputp    uputl          /* Copy user pointer type */
#define ugetp(x) ugetl(x)	/* between user and kernel */
#define uputi    uputl          /* Copy user int type */
#define ugeti(x) ugetl(x)	/* between user and kernel */

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)

extern void* memcpy(void*, const void*, size_t);
extern void* memset(void*, int, size_t);
extern size_t strlen(const char *);
extern uint16_t swab(uint16_t);

/* LX6 doesn't benefit from making a few key variables in
   non-reentrant functions static */
#define staticfast auto

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

#ifndef __STRINGIFY
#define __STRINGIFY(a) #a
#endif

#define __rsil(level) \
	(__extension__({ \
		uint32_t state; \
		__asm__ __volatile__( \
			"rsil %0," __STRINGIFY(level) : \
				"=a" (state) :: "memory"); \
		state; \
	}))

#define __hard_di() __rsil(15)
#define __hard_ei() __rsil(0)
#define __hard_irqrestore(ps) xtos_set_intlevel(ps)
	
#define no_cache_udata()

#define CPUTYPE	CPUTYPE_LX6

/* Memory helpers: Max of 32767 blocks (16MB) as written */
extern void copy_blocks(void *, void *, unsigned int);
extern void swap_blocks(void *, void *, unsigned int);

#ifndef __packed
#define __packed		__attribute__((packed))
#endif

#define barrier()		__asm__ volatile("":::"memory")

#define ntohs(x) ((uint16_t)(__builtin_bswap16((uint16_t)(x))))
#define ntohl(x) ((uint32_t)(__builtin_bswap32((uint32_t)(x))))

#define NORETURN __attribute__((__noreturn__))

#define __fastcall
