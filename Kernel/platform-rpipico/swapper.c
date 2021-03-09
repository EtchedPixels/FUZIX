#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>
#include "config.h"
#include "globals.h"

#undef DEBUG

/* A special swap and pagemap implementation for the Raspberry Pi Pico. Available
 * memory is divided into 4kB chunks; processes can occupy any number of these (up
 * to 64kB). When a process is switched in, memory is rearranged by swapping chunks
 * until the running process is at the bottom of memory in the right order.
 *
 * p->p_page is 0 if swapped out, 1 is swapped in.
 */

#define BLOCKSIZE 4096
#define NUM_ALLOCATION_BLOCKS (USERMEM / BLOCKSIZE)

uint8_t progbase[USERMEM];

struct mapentry
{
    uint8_t slot;
    uint8_t block;
};
static struct mapentry allocation_map[NUM_ALLOCATION_BLOCKS];

static uint8_t get_slot(ptptr p)
{
	uint8_t slot = p - ptab;
    if (slot >= PTABSIZE)
        panic("bad ptab");
}

static uaddr_t get_proc_size(ptptr p)
{
    if (!p)
        return 0;
    /* init is initially created with a p_top of 0, but it actually needs 512 bytes. */
    if (!p->p_top)
        p->p_top = PROGLOAD + 512;
    return p->p_top - PROGBASE;
}

static int get_proc_size_blocks(ptptr p)
{
    return (uaddr_t)alignup(get_proc_size(p), BLOCKSIZE) / BLOCKSIZE;
}

static struct mapentry* find_block(uint8_t slot, uint8_t block)
{
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if ((b->slot == slot) && (b->block == block))
            return b;
    }

    return NULL;
}

#ifdef DEBUG
    static void debug_blocks(void)
    {
        kprintf("current process size %p bytes %d blocks; isp %d rel\n",
            get_proc_size(udata.u_ptab), get_proc_size_blocks(udata.u_ptab),
            udata.u_isp - PROGBASE);
        for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
        {
            const struct mapentry* b = &allocation_map[i];
            void* p = (void*)PROGBASE + i*BLOCKSIZE;
            if (b->block != 0xff)
                kprintf("#%d: slot %d block %d %p\n", i, b->slot, b->block, p);
        }
    }
#endif

void pagemap_free(ptptr p)
{
    #ifdef DEBUG
        kprintf("free %d\n", get_slot(p));
    #endif
    int slot = get_slot(p);
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if (b->slot == slot)
        {
            #ifdef DEBUG
                kprintf("free slot #%d\n", i);
            #endif
            b->slot = b->block = 0xff;
        }
    }

	p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
	if (p == udata.u_ptab)
		return 0;

    int blocks = get_proc_size_blocks(p);
    int slot = get_slot(p);
    #ifdef DEBUG
        kprintf("alloc %d, %d blocks\n", get_slot(p), blocks);
    #endif

    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_block(0xff, 0xff); /* find a free block */
        if (!b)
            panic("ran out of memory");

        b->slot = slot;
        b->block = i;
    }

	p->p_page = 1;
    #ifdef DEBUG
        kprintf("done alloc\n");
        debug_blocks();
    #endif
	return 0;
}

/* size does *not* include udata */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
    struct p_tab* p = udata.u_ptab;

    uaddr_t oldblocks = get_proc_size_blocks(p);
    int blocks = (int)alignup(size + UDATA_SIZE, BLOCKSIZE) / BLOCKSIZE;
    int slot = get_slot(p);
    #ifdef DEBUG
        kprintf("realloc %d from %d to %d blocks\n", get_slot(udata.u_ptab), oldblocks, blocks);
    #endif
    if (blocks < oldblocks)
    {
        #ifdef DEBUG
            kprintf("shrinking process\n");
        #endif
        for (int i=blocks; i<oldblocks; i++)
        {
            struct mapentry* b = find_block(slot, i);
            b->slot = b->block = 0xff;
        }
    }
    else if (blocks > oldblocks)
    {
        #ifdef DEBUG
            kprintf("growing process\n");
        #endif
        for (int i=oldblocks; i<blocks; i++)
        {
            struct mapentry* b = find_block(0xff, 0xff);
            if (!b)
                panic("no memory in realloc");
            b->slot = slot;
            b->block = i;
        }
    }

    p->p_top = PROGBASE + blocks*BLOCKSIZE;
    #ifdef DEBUG
        debug_blocks();
    #endif
    contextswitch(p);
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return (PROGTOP - PROGBASE) >> 10;
}

void pagemap_init(void)
{
    #ifdef DEBUG
        kprintf("%d blocks of memory\n", NUM_ALLOCATION_BLOCKS);
    #endif
    memset(allocation_map, 0xff, sizeof(allocation_map));
	udata.u_ptab = NULL;
}

#if 0
/*
 *	Swap out the memory of a process to make room
 *	for something else
 */
int swapout_new(ptptr p, void *u)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;
    panic("swapout");

#ifdef DEBUG
	kprintf("Swapping out %d (%d)\n", get_slot(p), p->p_pid);
#endif
	if (!page)
		panic(PANIC_ALREADYSWAP);
	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and uarea etc..) to disk */

	/* Write the udata block as kernel. */
	udata.u_dptr = u;
	udata.u_block = blk;
	udata.u_nblock = UDATA_SIZE >> BLKSHIFT;	/* 1 */
	((*dev_tab[major(SWAPDEV)].dev_write) (minor(SWAPDEV), 0, 0));
	/* Use the standard swapwrite helper for the rest */
	swapwrite(SWAPDEV, blk + UDATA_BLKS, SWAPTOP - SWAPBASE,
		  SWAPBASE, 1);

	p->p_page = 0;
	p->p_page2 = map;
#ifdef DEBUG
	kprintf("%p: swapout done %d\n", p, p->p_page2);
#endif
	return 0;
}

int swapout(ptptr p)
{
	return swapout_new(p, &udata);
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */
void swapin(ptptr p, uint16_t map)
{
    panic("swapin");
	uint16_t blk = map * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %d (%d, %d)\n", get_slot(p), p->p_page2, p->p_pid);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	/* The udata might not be in common space read it as kernel mapped */
	udata.u_dptr = (uint8_t *)&udata;
	udata.u_block = blk;
	udata.u_nblock = UDATA_SIZE >> BLKSHIFT;
	/* The driver will use the data in udata before it writes over it */
	((*dev_tab[major(SWAPDEV)].dev_read) (minor(SWAPDEV), 0, 0));
	swapread(SWAPDEV, blk + UDATA_BLKS, SWAPTOP - SWAPBASE,
		 SWAPBASE, 1);

#ifdef DEBUG
	kprintf("%p: swapin done %d\n", p, p->p_page2);
#endif
}
#endif

void contextswitch(ptptr p)
{
    #ifdef DEBUG
        kprintf("context switch from %d to %d\n", get_slot(udata.u_ptab), get_slot(p));
    #endif

    int slot = get_slot(p);
    int blocks = get_proc_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b1 = &allocation_map[i];
        int i1 = b1 - allocation_map;
        void* p1 = (void*)PROGBASE + i1*BLOCKSIZE;
        if ((b1->slot != slot) || (b1->block != i))
        {
            struct mapentry* b2 = find_block(slot, i);
            int i2 = b2 - allocation_map;
            void* p2 = (void*)PROGBASE + i2*BLOCKSIZE;
            if (b1->slot == 0xff)
            {
                #ifdef DEBUG
                    kprintf("copy #%d to #%d\n", i2, i1);
                #endif
                memcpy(p1, p2, BLOCKSIZE);
            }
            else
            {
                #ifdef DEBUG
                    kprintf("swap #%d and #%d\n", i1, i2);
                #endif
                swap_blocks(p1, p2, BLOCKSIZE);
            }

            struct mapentry t = *b1;
            *b1 = *b2;
            *b2 = t;
        }
    }

    #ifdef DEBUG
        debug_blocks();
    #endif
}

/* Copy the current process into a new child slot, and context switch so it's live. */
void clonecurrentprocess(ptptr p)
{
    #ifdef DEBUG
        kprintf("clone %d to slot %d\n", get_slot(udata.u_ptab), get_slot(p));
        if (p->p_top != udata.u_ptab->p_top)
            panic("mismatched sizes");
    #endif
    int srcslot = get_slot(udata.u_ptab);
    int destslot = get_slot(p);
    int blocks = get_proc_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b1 = find_block(srcslot, i);
        struct mapentry* b2 = find_block(destslot, i);
        if (!b1 || !b2)
            panic("missing block");
        int i1 = b1 - allocation_map;
        int i2 = b2 - allocation_map;
        void* p1 = (void*)PROGBASE + i1*BLOCKSIZE;
        void* p2 = (void*)PROGBASE + i2*BLOCKSIZE;
        #ifdef DEBUG
            kprintf("copy #%d to #%d (%p to %p)\n", i1, i2, p1, p2);
        #endif
        memcpy(p2, p1, BLOCKSIZE);

        struct mapentry t = *b1;
        *b1 = *b2;
        *b2 = t;
    }
    #ifdef DEBUG
        kprintf("end clone\n");
    #endif
}

uint_fast8_t platform_canswapon(uint16_t devno)
{
    /* Only allow swapping to hd devices. */
    return (devno >> 8) == 0;
}

// vim: ts=4 sw=4 et

