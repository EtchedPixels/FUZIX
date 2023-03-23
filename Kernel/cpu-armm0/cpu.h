#include <string.h>

#define ALIGNUP(v)   alignup(v, 4)
#define ALIGNDOWN(v) aligndown(v, 4)

#define uputp    uputl          /* Copy user pointer type */
#define ugetp(x) ugetl(x)       /* between user and kernel */
#define uputi    uputl          /* Copy user int type */
#define ugeti(x) ugetl(x)	/* between user and kernel */

#define brk_limit() (udata.u_ptab->p_top)

extern void* memcpy(void*, const void*, size_t);
extern void* memset(void*, int, size_t);
extern size_t strlen(const char *);
extern uint16_t swab(uint16_t);

/* ARM M0 doesn't benefit from making a few key variables in
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

inline static uint32_t __hard_di(void)
{
	uint32_t ps;
	asm volatile("mrs %0, PRIMASK" : "=r" (ps));
	asm volatile("cpsid i");
	return ps;
}

inline static void __hard_ei(void)
{
	asm volatile("cpsie i");
}

inline static void __hard_irqrestore(uint32_t ps)
{
	asm volatile("msr PRIMASK, %0" :: "r" (ps));
}

#define barrier() asm volatile("":::"memory")
	
/* jmp over the Fuzix header. Will need updating if the header size changes */
#define EMAGIC   0x08
#define EMAGIC_2 0x3c

#define no_cache_udata()

#define ntohs(x) ((uint16_t)(__builtin_bswap16((uint16_t)(x))))
#define ntohl(x) ((uint32_t)(__builtin_bswap32((uint32_t)(x))))

#define CPUTYPE	CPUTYPE_ARMM0

/* Memory helpers: Max of 32767 blocks (16MB) as written */
extern void copy_blocks(void *, void *, unsigned int);
extern void swap_blocks(void *, void *, unsigned int);

#define NORETURN __attribute__((__noreturn__))
#undef __fastcall
#define __fastcall /* nothing */

