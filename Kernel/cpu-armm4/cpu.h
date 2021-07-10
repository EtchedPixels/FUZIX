#ifndef __ARMM4_CPU_H
#define __ARMM4_CPU_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "types.h"

#include "config.h"

#ifdef CONFIG_INLINE_IRQ
#include <inline-irq.h>
#else /* CONFIG_INLINE_IRQ */
#error Only inline IRQ functions available.
#endif /* CONFIG_INLINE_IRQ */

#ifdef CONFIG_SOFT_IRQ
#error Soft IRQ not supported.
#endif

#define ALIGNUP(v)   alignup((v), 4U)
#define ALIGNDOWN(v) aligndown((v), 4U)

#define uputp    uputl            /* Copy user pointer type */
#ifdef ELF32
#define ugetp(x) ugetl((x), NULL) /* between user and kernel */
#else /* ELF32 */
#define ugetp    ugetl            /* between user and kernel */
#endif /* ELF32 */
#define uputi    uputl            /* Copy user int type */
#define ugeti(x) ugetl((x), NULL) /* between user and kernel */

#ifdef CONFIG_FLAT

struct u_data;
extern struct u_data *udata_ptr;

#define udata (*(udata_ptr))

#else /* CONFIG_FLAT */
#error Only flat memory currently supported.
#endif /* CONFIG_FLAT */

#define no_cache_udata()

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (((udata).u_syscall_sp) - 512)

void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
size_t strlen(const char *);
uint16_t swab(uint16_t);

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

#define getreg8(a)     (*((volatile uint8_t *)(a)))
#define putreg8(v, a)  (*((volatile uint8_t *)(a)) = (v))
#define getreg16(a)    (*((volatile uint16_t *)(a)))
#define putreg16(v, a) (*((volatile uint16_t *)(a)) = (v))
#define getreg32(a)    (*((volatile uint32_t *)(a)))
#define putreg32(v, a) (*((volatile uint32_t *)(a)) = (v))

/* Non-atomic modification of registers */
#define modreg8(v, m, a)  putreg8((getreg8(a) & ~(m)) | ((v) & (m)), (a))
#define modreg16(v, m, a) putreg16((getreg16(a) & ~(m)) | ((v) & (m)), (a))
#define modreg32(v, m, a) putreg32((getreg32(a) & ~(m)) | ((v) & (m)), (a))

#define cpu_to_le16(x) (x)
#define le16_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define le32_to_cpu(x) (x)

#define ntohs(x) ((uint16_t)(__builtin_bswap16((uint16_t)(x))))
#define ntohl(x) ((uint32_t)(__builtin_bswap32((uint32_t)(x))))

void copy_blocks(void *, void *, size_t);
void swap_blocks(void *, void *, size_t);

#define CPUTYPE CPUTYPE_ARMM4

#endif /* __ARMM4_CPU_H */
