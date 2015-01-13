typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef signed int size_t;

typedef uint8_t irqflags_t;

typedef int16_t arg_t;
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint16_t uaddr_t;

extern void ei(void);
extern irqflags_t di(void);
extern void __fastcall__ irqrestore(irqflags_t f);

extern void * __fastcall__ memcpy(void *, void *, size_t);
extern void * __fastcall__ memset(void *, int, size_t);
extern size_t __fastcall__ strlen(const char *);

#define EMAGIC    0x4C    /* Header of executable (JMP) */
#define brk_limit() ramtop	/* Stack is preallocated */

#define staticfast	static

/* User's structure for times() system call */
typedef unsigned long clock_t;

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

/* We don't yet have bank attributes and banking for 6502 */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD
