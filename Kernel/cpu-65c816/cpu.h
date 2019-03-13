/*
 *	We treat the 65c816 as a glorified 6502.
 *	Processes are executed in 65c816 mode with their own 64K bank set by
 *	the bank registers. The CPU stack is 256 bytes in low 64K, and the
 *	direct page likewise.
 *
 *	Because cc65 stores temporaries and return addresses on the CPU stack,
 *	and uses ZP for register variables (whom in C you cannot take the
 *	address of) this works fine for CC65 apps and the kernel likewise only
 *	has to worry about 16bit pointers except for user copies and asm bits.
 */

typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int size_t;
typedef signed int ssize_t;

/* Whilst we are using cc65 anyway */
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

extern void * __fastcall__ memcpy(void *, void *, size_t);
extern void * __fastcall__ memset(void *, int, size_t);
extern size_t __fastcall__ strlen(const char *);

#define EMAGIC    0x4C    /* Header of executable (JMP) */
#define EMAGIC_2  0x38	  /* SEC BCS foo */
/* We use SEC BCS not CLC BCC because CLC is 0x18 which is the Z80 JR header
   so the two would be identical - not good! */

#define staticfast	static

/* Handled with an asm helper on this processor due to the dual stacks */
extern uint16_t brk_limit(void);

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

/* No support for inline */
#define inline

/* FIXME: should swap a/b inline ??? */
#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))

#define CPUTYPE	CPUTYPE_65C816

/* cc65 really wants structs used repeatedly to be marked register */
#define regptr	register

#define __packed
#define barrier()
