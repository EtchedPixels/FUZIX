typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned long size_t;

typedef uint16_t irqflags_t;

typedef int32_t arg_t;
typedef uint32_t uarg_t;		/* Holds arguments */
typedef uint32_t usize_t;		/* Largest value passed by userspace */
typedef int32_t susize_t;
typedef uint32_t uaddr_t;

extern void ei(void);
extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);

extern void *memcpy(void *, const void  *, size_t);
extern void *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);
extern size_t strlen(const char *);

#define EMAGIC    0x4C    /* Header of executable (JMP) */
#define EMAGIC_2  0x38	  /* SEC BCS foo */

#define brk_limit() ((udata.u_syscall_sp) - 512)

#define staticfast

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef struct {
  uint32_t low;
  uint32_t high;
} time_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t high;
      uint16_t low;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;

/* We don't need any banking bits really */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD

/* Pointers are 32bit */
#define POINTER32

/* Sane behaviour for unused parameters */
#define used(x)

#define __fastcall__

/* Our udata is handled slightly quirkily - use a register global */

register struct u_data *udata_ptr asm ("a5");

#define udata (*udata_ptr)
