#define CPU_MID	MID_FUZIXNS32

#define uputp  uputl			/* Copy user pointer type */
#define ugetp(x)  ugetl(x)		/* between user and kernel */
#define uputi  uputl			/* Copy user int type */
#define ugeti(x)  ugetl(x)		/* between user and kernel */

extern void *memcpy(void *, const void  *, size_t);
extern void *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);
extern size_t strlen(const char *);

#define brk_limit() ((udata.u_syscall_sp) - 512)

#define staticfast

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef union {            /* this structure is endian dependent */
    clock_t  full;         /* 32-bit count of ticks since boot */
    struct {
      uint16_t high;
      uint16_t low;         /* 16-bit count of ticks since boot */
    } h;
} ticks_t;

extern uint16_t swab(uint16_t);

#define ntohs(x)	((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ntohl(x)	((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | \
                         (((x) & 0xFF0000) >> 8) | (((x >> 24) & 0xFF)))

#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)

/* Pointers are 32bit */
#define POINTER32
typedef uint32_t paddr_t;

/* Sane behaviour for unused parameters */
#define used(x)

/* Only for speed reasons */
#define ALIGNUP(v)	(((v) + 3) & ~3)
#define ALIGNDOWN(v)	((v) & ~3)
#define STACKALIGN(v)	((v) & ~3)

/* Our udata is handled slightly quirkily - use a register global */

register struct u_data *udata_ptr asm ("r6");

#define udata (*udata_ptr)

#define CONFIG_STACKSIZE	1024

#define __packed		__attribute__((packed))
#define barrier()		asm volatile("":::"memory")
#define NORETURN		__attribute__((__noreturn__))

/* Memory helpers: Max of 32767 blocks (16MB) as written */
extern void copy_blocks(void *, void *, unsigned int);
extern void swap_blocks(void *, void *, unsigned int);

extern void *memcpy32(void *to, const void *from, size_t bytes);

extern int probe_memory(void *p);
extern int cpu_type(void);

#define __fastcall
