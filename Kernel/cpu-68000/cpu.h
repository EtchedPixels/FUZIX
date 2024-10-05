#define CPU_MID MID_FUZIX68000

#define uputp  uputl			/* Copy user pointer type */
#define ugetp(x)  ugetl(x)		/* between user and kernel */
#define uputi  uputl			/* Copy user int type */
#define ugeti(x)  ugetl(x)		/* between user and kernel */

extern void *memcpy(void *, const void  *, size_t);
extern void *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);
extern size_t strlen(const char *);
extern int strcmp(const char *, const char *);

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

#define	ntohs(x)	(x)
#define ntohl(x)	(x)

#define cpu_to_le16(x)	le16_to_cpu(x)
#define le16_to_cpu(x)	(uint16_t)(__builtin_bswap16((uint16_t)(x)))
#define cpu_to_le32(x)	le32_to_cpu(x)
#define le32_to_cpu(x)	(uint32_t)(__builtin_bswap32((uint32_t)(x)))

/* Pointers are 32bit */
#define POINTER32

typedef	uint32_t	paddr_t;	/* 32bit physical addresses */
typedef uint16_t	page_t;		/* Page frame numbering */

/* Sane behaviour for unused parameters */
#define used(x)

/* Our udata is handled slightly quirkily - use a register global */

register struct u_data *udata_ptr asm ("a5");

#define udata (*udata_ptr)

#define BIG_ENDIAN

#define CONFIG_STACKSIZE	1024

#define __packed		__attribute__((packed))
#define barrier()		asm volatile("":::"memory")

/* Memory helpers: Max of 32767 blocks (16MB) as written */
extern void copy_blocks(void *, void *, unsigned int);
extern void swap_blocks(void *, void *, unsigned int);

extern void *memcpy32(void *to, const void *from, size_t bytes);

extern int probe_memory(void *p);
extern int cpu_type(void);

extern void enable_icache(void);

/* Optional mapping helpers for I/O memory windows when we have portable
   driver code. The platform defines IOMAP(x) to return the value of the
   memory location for that I/O port */

#define IOMEM(x)		((volatile uint8_t *)IOMAP((uint8_t)x))
#define IOMEM16(x)		((volatile uint8_t *)IOMAP(x))

#define in(x)			(*IOMEM(x))
#define out(x,y)		do { *IOMEM(x) = (y); } while(0)
/* 16bit port numbers with 8bit data - not 16bit data */
#define in16(x)			(*IOMEM16(x))
#define out16(x,y)		do { *IOMEM16(x) = (y); } while(0)

/* We require word alignment */
#define UNALIGNED(x)		((x) & 1)
#define ALIGNUP(x)		(((x) + 1) & ~1)
#define ALIGNDOWN(x)		((x) & ~1)
/* Stack 32bit aligned for speed on later processors */
#define STACKALIGN(x)		((x) & ~3)

/* In a 32bit environment udata.u_codebase is the progrram base for brk */
#define PROGBASE		(udata.u_codebase)

#define __fastcall
#define NORETURN __attribute__((__noreturn__))
