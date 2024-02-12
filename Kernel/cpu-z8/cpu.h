
#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

typedef uint16_t irqflags_t;

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)

typedef uint16_t size_t;
typedef int16_t ssize_t;
extern void *memcpy(void *dest, const void *src, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern char *strchr(const char *s, int c);
extern void *memset(void *dest, int c, size_t n);

/* There is no compiler optimized inline memmove */
extern void *memmove(void *dest, const void *src, size_t n);

extern int16_t strlen(const char *p);

#define	staticfast

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t low;         /* 16-bit count of ticks since boot */
      uint16_t high;
    } h;
} ticks_t;

#define used(x)	x

#define cpu_to_le16(x)	swab(x)
#define le16_to_cpu(x)	swab(x)
#define cpu_to_le32(x)	((((uint32_t)cpu_to_le16((x) & 0xFFFF)) << 16) | \
				(uint32_t)cpu_to_le16((x) >> 16))
#define le32_to_cpu(x)	cpu_to_le32(x)

#define ntohs(x)	(x)
#define ntohl(x)	(x)

#define BIG_ENDIAN

#define __packed
#define barrier()
#define inline

#define __fastcall

extern void out(uint_fast8_t port, uint_fast8_t val);
extern uint_fast8_t in(uint_fast8_t port);
