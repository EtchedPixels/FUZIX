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

extern void ei(void);
extern irqflags_t di(void);
extern void irqrestore(irqflags_t f);

#define EMAGIC    0x0E    /* Header of executable  (JMP) */

extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
extern size_t strlen(const char *);
extern uint16_t swab(uint16_t);

/* 6809 doesn't benefit from making a few key variables in
   non-reentrant functions static */
#define staticfast	auto

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


#define cpu_to_le16(x)	swab(x)
#define le16_to_cpu(x)	swab(x)

#ifdef CONFIG_BANKED
#define CODE1	__attribute__((far("1")))
#define CODE2   __attribute__((far("2")))
#define COMMON
#define DISCARD __attribute__((far("3")))
#define VIDEO   __attribute__((far("4")))
#else
/* Bank attributes for 6809 in banked code mode */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD
#endif
