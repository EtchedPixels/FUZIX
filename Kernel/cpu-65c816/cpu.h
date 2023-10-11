/*
 *	65C816 native mode. We run mostly in 16bit mode so we look like
 *	any other normal littl endian 8/16bit CPU setup
 */

#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

extern void * memcpy(void *, void *, size_t);
extern void * memset(void *, int, size_t);
extern size_t strlen(const char *);

#define staticfast

/* Handled with an asm helper on this processor due to the dual stacks */
extern uint16_t brk_limit(void);

/* User's structure for times() system call */
typedef unsigned long clock_t;

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

#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))

#define le16_to_cpu(x)	(x)
#define le32_to_cpu(x)	(x)
#define cpu_to_le16(x)	(x)
#define cpu_to_le32(x)	(x)
#define regptr

#define __packed
#define barrier()

#define __fastcall
