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


extern void *memcpy(void *, void *, size_t);
extern void *memset(void *, int, size_t);
extern size_t strlen(const char *);

/* 6809 doesn't benefit from making a few key variables in
   non-reentrant functions static */
#define staticfast	auto

/* FIXME: should be 64bits - need to add helpers and struct variants */
typedef unsigned long long time_t;

#ifdef CONFIG_BANKED
#define CODE1	__attribute__((far(1)))
#define CODE2   __attribute__((far(2)))
#define COMMON
#define DISCARD __attribute__((far(3)))
#define VIDEO   __attribute__((far(4)))
#else
/* Bank attributes for 6809 in banked code mode */
#define CODE1
#define CODE2
#define COMMON
#define VIDEO
#define DISCARD
#endif
