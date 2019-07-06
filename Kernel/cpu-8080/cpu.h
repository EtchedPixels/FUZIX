typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

/* ACK thinks in 16bit chunks and extends 8bit when working internally. All our
   8bit indexes etc give it indigestion */

typedef unsigned short uint_fast8_t;
typedef signed short int_fast8_t;

typedef int16_t  arg_t;			/* Holds arguments */
typedef uint16_t uarg_t;		/* Holds arguments */
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint16_t uaddr_t;		/* User address */
typedef uint16_t uptr_t;		/* Userspace pointer equivalent */

#define MAXUSIZE	0xFFFF

#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

typedef uint16_t irqflags_t;

/* 8080 binaries start with a JP */
#define EMAGIC    0xc3    /* Header of executable */
#define EMAGIC_2  0xc3	  /* JR */
/* Allow a minimum of 512 bytes gap between stack and top of allocations */
#define brk_limit() (udata.u_syscall_sp - 512)

typedef uint16_t size_t;
typedef uint16_t ssize_t;
extern void *memcpy(void *dest, const void *src, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern char *strchr(const char *s, int c);
extern void *memset(void *dest, int c, size_t n);

/* There is no compiler optimized inline memmove */
extern void *memmove(void *dest, const void *src, size_t n);

extern int16_t strlen(const char *p);

#define	staticfast	static

/* User's structure for times() system call */
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

#define used(x)	x

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)

#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ntohl(x)	((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | \
                         (((x) & 0xFF0000) >> 8) | (((x >> 24) & 0xFF)))

#define CPUTYPE	CPUTYPE_8080

/* Deal with SDCC code gen issue */
#define HIBYTE32(x)	(((uint8_t *)&(x))[3])

/* ack8080 does not support attribute(packed) but then it also doesn't insert
   padding either */

#define __packed
#define barrier()
#define inline
