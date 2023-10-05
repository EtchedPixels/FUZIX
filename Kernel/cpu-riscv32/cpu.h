#ifndef __RISCV32_CPU_H
#define __RISCV32_CPU_H

/*
 *	This is just a fudge to get us started. It needs updating once
 *	we get stuff building as our I/O space is different.
 */
#include <stddef.h>
#include <stdint.h>

#include "types.h"
#include "config.h"

extern irqflags_t __hard_di(void);
extern void __hard_ei(void);
extern void __hard_irqrestore(irqflags_t mie);

#ifdef CONFIG_SOFT_IRQ
#error Soft IRQ not supported.
#endif

#define ALIGNUP(v)   alignup((v), 4U)
#define ALIGNDOWN(v) aligndown((v), 4U)
#define STACKALIGN(v) aligndown((v), 16U)

#define uputp    uputl          /* Copy user pointer type */
#define ugetp(x) ugetl(x)	/* between user and kernel */
#define uputi    uputl          /* Copy user int type */
#define ugeti(x) ugetl(x)	/* between user and kernel */

#ifdef CONFIG_FLAT

register struct u_data *udata_ptr asm ("tp");

struct u_data;
extern struct u_data *udata_ptr;

#define udata (*(udata_ptr))

#else /* CONFIG_FLAT */
#error Only flat memory currently supported.
#endif /* CONFIG_FLAT */

#define no_cache_udata()

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (((udata).u_syscall_sp) - 512)

int memcmp(const void *a, const void *b, size_t n);
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
size_t strlen(const char *);
uint16_t swab(uint16_t);

#define staticfast auto

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t low;
      uint16_t high;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;

#define used(x)

/* Pointers are 32bit */
#define POINTER32
#undef FAR
#define FAR

#undef  BIG_ENDIAN
#define LITTLE_ENDIAN

#ifndef __packed
#define __packed  __attribute__((packed))
#endif

#define barrier() asm volatile("":::"memory")

/* Our I/O is memory mapped. Provide equivalent functions. Note that we
   follow the sane ordering in out() not the DOS one */
#define in(a)		(*((volatile uint8_t *)(a)))
#define out(a, v)	(*((volatile uint8_t *)(a)) = (v))
#define inw(a)		(*((volatile uint16_t *)(a)))
#define outw(a, v)	(*((volatile uint16_t *)(a)) = (v))
#define inl(a)		(*((volatile uint32_t *)(a)))
#define outl(a, v)	(*((volatile uint32_t *)(a)) = (v))

/* Non-atomic modification of registers */
#define outmod8(a, m, v)  out((a),  (in(a) & ~(m)) | ((v) & (m)))
#define outmod16(a, m, v) outw((a), (inw(a) & ~(m)) | ((v) & (m)))
#define outmod32(a, m, v) outl((a), (inl(a) & ~(m)) | ((v) & (m)))

#define cpu_to_le16(x) (x)
#define le16_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define le32_to_cpu(x) (x)

#define ntohs(x) ((uint16_t)(__builtin_bswap16((uint16_t)(x))))
#define ntohl(x) ((uint32_t)(__builtin_bswap32((uint32_t)(x))))
#define htons(x) ((uint16_t)(__builtin_bswap16((uint16_t)(x))))
#define htonl(x) ((uint32_t)(__builtin_bswap32((uint32_t)(x))))

void copy_blocks(void *, void *, size_t);
void swap_blocks(void *, void *, size_t);

#define NORETURN __attribute__((__noreturn__))

#define CPUTYPE CPUTYPE_RISCV32
#define CPU_MID MID_RISCV32

#endif /* __RISCV32_CPU_H */
