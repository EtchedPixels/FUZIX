typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int size_t;
typedef signed int ssize_t;

typedef unsigned char uint_fast8_t;
typedef signed char int_fast8_t;

typedef uint8_t irqflags_t;

typedef int16_t arg_t;
typedef uint16_t uarg_t;		/* Holds arguments */
typedef uint16_t usize_t;		/* Largest value passed by userspace */
typedef int16_t susize_t;
typedef uint16_t uaddr_t;
typedef uint16_t uptr_t;		/* User pointer equivalent */

#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

extern void *  memcpy(void *, const void *, size_t);
extern void *  memset(void *, int, size_t);
extern size_t  strlen(const char *);

/* High byte is saved, low byte is a mystery so take worst case. Also allow
   a bit less as C stack is not return stack */
#define brk_limit() ((((uint16_t)udata.u_syscall_sp) | 0xFF) - 384)

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
      uint16_t low;         /* 16-bit count of ticks since boot */
      uint16_t high;
    } h;
} ticks_t;

/* Sane behaviour for unused parameters */
#define used(x)

typedef uint8_t page_t;		/* May need to change this for 286PM */

struct proc_map {
	uint16_t cseg;		/* CS to use. Keep computed for asm code */
	uint16_t dseg;		/* DS to use. Ditto */
#ifdef CONFIG_IBMPC_EMM
	uint16_t emm;		/* 0 - none, 1+ = bank code, and the emm
				   frame is defined by dseg so that the core
				   code is mostly oblivious to it */
#endif
	page_t cbase;		/* The rest is only used in C code */
	uint8_t csize;
	page_t dbase;
	uint8_t dsize;
	uint8_t ssize;
};

extern uint16_t get_code_segment(uint16_t pv);
extern uint16_t get_data_segment(uint16_t pv);

extern void cpu_detect(void);

static inline void outb(uint8_t value, uint16_t port)
{
	register uint16_t portv asm("dx") = port;
	register uint8_t valv asm("al") = value;
	asm volatile("outb %0, %1" : : "Na"(valv), "Nd"(portv));
}

static inline uint8_t inb(uint16_t port)
{
	register uint8_t value asm("al");
	register uint16_t portv asm("dx") = port;
	asm volatile("inb %1, %0" : "=a"(value) : "Nd"(portv));
	return value;
}

#define __packed		__attribute__((packed))
#define barrier()		asm volatile("":::"memory")

#define __fastcall
