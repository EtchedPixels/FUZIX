
#define uputp  uputw			/* Copy user pointer type */
#define ugetp  ugetw			/* between user and kernel */
#define uputi  uputw			/* Copy user int type */
#define ugeti  ugetw			/* between user and kernel */

extern void * __fastcall__ memcpy(void *, void *, size_t);
extern void * __fastcall__ memset(void *, int, size_t);
extern size_t __fastcall__ strlen(const char *);

/* High byte is saved, low byte is a mystery so take worst case. Also allow
   a bit less as C stack is not return stack */
#define brk_limit() ((((uint16_t)udata.u_syscall_sp) | 0xFF) - 384)

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

/* No useful behaviour for unused parameters */
#define used(x)

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)

/* No support for inline */
#define inline

#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))

/* cc65 really wants register tags on struct pointers used repeatedly */
#define regptr	register

/* CC65 is bright enough to partly optimise this but not fully so do it
   by hand */
#define HIBYTE32(x)	(((uint8_t *)&(x))[3])

#define __packed
#define barrier()

#define __fastcall
