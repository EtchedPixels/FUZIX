typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef int16_t  arg_t;			/* Holds arguments */
typedef uint16_t uarg_t;		/* Holds arguments */
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint16_t uaddr_t;		/* User address */
typedef uint16_t uptr_t;		/* Userspace pointer equivalent */

#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */

#define ei()   do {__asm ei __endasm; } while(0);

typedef uint16_t irqflags_t;

extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);
extern void out(uint8_t addr, uint8_t val);
extern uint8_t in(uint8_t addr);

/* Z80 binaries start with a JP */
#define EMAGIC    0xc3    /* Header of executable */
#define EMAGIC_2  0x18	  /* JR */
/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)


/* compiler provides optimised versions of these: */
#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka)
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