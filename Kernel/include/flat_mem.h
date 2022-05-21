/*
 *	Flat memory manager taken from the original MAPUX work by
 *	Martin Young. "Do what you want with it" licence.
 *
 *	This version has been tidied up and ANSIfied
 */

#ifndef _HAVE_MEMMAN_H_
#define _HAVE_MEMMAN_H_

/* Choose BLKSIZE so that the full memory range fits into an unsigned int.
   The bigger the block size the less overhead and the more wastage. If your
   CPU lacks a fast multiply stick to powers of two */
#ifndef MMU_BLKSIZE
#define MMU_BLKSIZE		8192
#endif

/* These are much too big. We need to split virtual and physical management
   and pull some other tricks to get the size down. At the moment with 8K
   pages it costs us 2.5K / MB of RAM */
struct memblk
{
	struct memblk	*next;		/* Next allocation block */
	struct memblk	*prev;		/* Previous allocation block */
	uint8_t		*vaddr;		/* Address when in use */
	struct memblk	*home;		/* Owner */
	size_t		gap;		/* Virtual address space we want
					   to reserve beyond this block */
};

struct mmu_context {
	struct memblk mmu;
	uint8_t *base;
};

extern void vmmu_init(void);
extern void *vmmu_alloc(struct memblk *list, size_t size, void *addr, size_t resv, int safe);
extern void vmmu_setcontext(struct memblk *list);
extern void vmmu_free(struct memblk *list);
extern int vmmu_dup(struct memblk *list, struct memblk *dest);

extern int pagemap_fork(ptptr p);

/* Platform provided. Must be suitably aligned */
extern uint8_t *membase;
extern uint8_t *memtop;
/* Fast BLKSIZE and nicely aligned transfer functions */
extern void fast_zero_block(void *p);
extern void fast_swap_block(void *p, void *q);
extern void fast_copy_block(void *s, void *d);
/* Hook to allow the batching of operations on machines with handy things
   like a blitter that copies faster than the CPU */
extern void fast_op_complete(void);

extern void plt_mmu_setup(struct mmu_context *m);

#endif

