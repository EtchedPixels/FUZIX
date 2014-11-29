typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef int16_t arg_t;			/* Holds arguments */
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;

#define ei()   do {__asm ei __endasm; } while(0);

typedef uint16_t irqflags_t;

extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);

/* Z80 binaries start with a JP */
#define EMAGIC    0xc3    /* Header of executable */

/* compiler provides optimised versions of these: */
#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka)
#define memcpy(dst, src, n) __builtin_memcpy(dst, src, n)
#define strcpy(dst, src) __builtin_strcpy(dst, src)
#define strncpy(dst, src, n) __builtin_strncpy(dst, src, n)
#define strchr(s, c) __builtin_strchr(s, c)
#define memset(dst, c, n) __builtin_memset(dst, c, n)

extern int16_t strlen(const char *p);

#define	staticfast	static

/* Must match native ordering of long long */
typedef struct {
	uint32_t low;
	uint32_t high;
} time_t;

/* We don't yet have bank attributes and banking for Z80 */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)

#endif
