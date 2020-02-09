/* The MSP430X has 20-bit registers. Yes, really. It defines size_t
 * to be one of those. Use that rather than uint32_t because it's
 * way more efficient, but 20-bit pointers are still stored in memory
 * as 32-bit values.
 *
 * However, we only build the kernel in 20-bit mode; userland is in
 * 16-bit mode. Because all userspace addresses are below 64kB, it's
 * safe to cast a 16-bit value to a 20-bit address and vice verse.
 */

typedef uintptr_t uint20_t;
typedef intptr_t int20_t;

typedef uint8_t irqflags_t;

typedef uint16_t arg_t;
typedef uint16_t uarg_t;
typedef uint16_t uaddr_t;
typedef uint16_t uptr_t;
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint32_t clock_t;

#define MAXUSIZE	0xFFFF

/* The MSP430 requires aligned accesses. (Annoying, it doesn't trap if you
 * get this wrong. It just reads to or writes from the wrong place.) */

#define ALIGNUP(v)   alignup(v, 2)
#define ALIGNDOWN(v) aligndown(v, 2)

#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

#define ei() \
	asm volatile ("eint")

#if 0
static inline irqflags_t di(void)
{
	irqflags_t flags;
	asm volatile (
		"mov SR, %0\n"
		"dint\n"
		: "=g" (flags));
	return flags;
}
#endif

#if 0
static inline void irqrestore(irqflags_t flags)
{
	asm volatile (
		"dint\n"
		"bis.w %0, SR\n"
		: "+g" (flags));
}
#endif

extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)

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

#define __read_hidata(p) \
	({ \
		uint8_t r; \
		asm ("movx.b 0x10000(%1), %0" \
			: "=g" (r) \
			: "r" (p)); \
		r; \
	})

#define __write_hidata(p, v) \
	({ \
		asm volatile ("movx.b %0, 0x10000(%1)" \
			: \
			: "g" (v), "r" (p)); \
	})

#define used(x)

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_le32(x)  (x)
#define le32_to_cpu(x)  (x)

#define no_cache_udata()

#define __packed		__attribute__((packed))
#define barrier()		asm volatile("":::"memory")

#define __fastcall
