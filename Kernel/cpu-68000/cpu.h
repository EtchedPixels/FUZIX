#define uputp  uputl			/* Copy user pointer type */
#define ugetp  ugetl			/* between user and kernel */
#define uputi  uputl			/* Copy user int type */
#define ugeti(x)  ugetl(x,NULL)		/* between user and kernel */

extern void *memcpy(void *, const void  *, size_t);
extern void *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);
extern size_t strlen(const char *);

#define brk_limit() ((udata.u_syscall_sp) - 512)

#define staticfast

/* User's structure for times() system call */
typedef unsigned long clock_t;

typedef struct {
  uint32_t high;
  uint32_t low;
} time_t;

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

#define __fastcall
