#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

typedef uint16_t irqflags_t;

extern void out(uint8_t addr, uint8_t val);
extern uint8_t in(uint8_t addr) __z88dk_fastcall;

/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)


/* compiler provides optimised versions of these: */
/* FIXME: review re LDIR on r2k */
#if defined(__SDCC_r2k) || defined(__SDCC_r3ka)
#define memcpy(dst, src, n) __builtin_memcpy(dst, src, n)
#define strcpy(dst, src) __builtin_strcpy(dst, src)
#define strncpy(dst, src, n) __builtin_strncpy(dst, src, n)
#define strchr(s, c) __builtin_strchr(s, c)
#define memset(dst, c, n) __builtin_memset(dst, c, n)
#else
typedef uint16_t size_t;
extern void *memcpy(void *dest, const void *src, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern char *strchr(const char *s, int c);
extern void *memset(void *dest, int c, size_t n);
#endif

extern int16_t strlen(const char *p);

#define	staticfast	static/* User's structure for times() system call */
typedef unsigned long clock_t;

/* Must match native ordering of long long */
typedef struct {
	uint32_t low;
	uint32_t high;
} time_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t low;         /* 16-bit count of ticks since boot */
      uint16_t high;
    } h;
} ticks_t;

/* SDCC is arsy about unused parameters */
#define used(x)	x

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)

/* Do not use COMMON_MEMORY except for __asm code blocks. The SDCC helpers are not
   loaded into common */
#define COMMON_MEMORY   static void COMMONSEG(void)  __naked { __asm .area _COMMONMEM __endasm; }

#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ntohl(x)	((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | \
                         (((x) & 0xFF0000) >> 8) | (((x >> 24) & 0xFF)))

/* Deal with SDCC code gen issue */
#define HIBYTE32(x)	(((uint8_t *)&(x))[3])

/* SDCC does not support attribute(packed) but then it also doesn't insert
   padding either */

#define __packed
#define barrier()

#define __fastcall
