#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>
#include "config.h"
#include "globals.h"

#undef DEBUG

/* A special swap and pagemap implementation for the ESP8266. Available
 * memory is divided into 4kB chunks; processes can occupy any number of these (up
 * to 64kB). When a process is switched in, memory is rearranged by swapping chunks
 * until the running process is at the bottom of memory in the right order.
 *
 * p->p_page is 0 if swapped out, 1 is swapped in.
 */

#define BLOCKSIZE 512
#define NUM_DATA_ALLOCATION_BLOCKS (DATALEN / BLOCKSIZE)
#define NUM_CODE_ALLOCATION_BLOCKS (CODELEN / BLOCKSIZE)
#define NUM_EXTRA_ALLOCATION_BLOCKS (16*1024 / BLOCKSIZE)
#define NUM_ALLOCATION_BLOCKS (NUM_DATA_ALLOCATION_BLOCKS \
    + NUM_CODE_ALLOCATION_BLOCKS + NUM_EXTRA_ALLOCATION_BLOCKS)

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
    return slot;
}

static uaddr_t get_address(int block)
{
    if (block < NUM_DATA_ALLOCATION_BLOCKS)
        return DATABASE + BLOCKSIZE*block;
    block -= NUM_DATA_ALLOCATION_BLOCKS;
    if (block < NUM_CODE_ALLOCATION_BLOCKS)
        return CODEBASE + BLOCKSIZE*block;
    block -= NUM_CODE_ALLOCATION_BLOCKS;
    return 0x40108000 + BLOCKSIZE*block;
}

static uaddr_t get_proc_data_size(ptptr p)
{
    if (!p)
        return 0;
    /* init is initially created with a p_top of 0, but it actually needs 512 bytes. */
    if (!p->p_top)
        udata.u_top = p->p_top = DATABASE + 512;
    return p->p_top - DATABASE;
}

static uaddr_t get_proc_code_size(ptptr p)
{
    if (!p)
        return 0;
    /* init is initially created with a p_texttop of 0. */
    if (!p->p_texttop)
        udata.u_texttop = p->p_texttop = CODEBASE;
    return p->p_texttop - CODEBASE;
}

static int get_proc_data_size_blocks(ptptr p)
{
    return (uaddr_t)alignup(get_proc_data_size(p), BLOCKSIZE) / BLOCKSIZE;
}

static int get_proc_code_size_blocks(ptptr p)
{
    return (uaddr_t)alignup(get_proc_code_size(p), BLOCKSIZE) / BLOCKSIZE;
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

static struct mapentry* find_free_block(ptptr p)
{
    for (;;)
    {
        struct mapentry* b = find_block(0xff, 0xff); /* find a free block */
        if (b)
            return b;

        #ifdef DEBUG
            kprintf("alloc failed, finding a process to swap out");
        #endif
        if (!swapneeded(p, true))
        {
            kprintf("warning: out of memory\n");
            return NULL;
        }
    }
}

#ifdef DEBUG
    static void debug_blocks(void)
    {
        kprintf("current process size:\n  code %p bytes %d blocks;\n  data %p bytes %d blocks;\n  isp %x rel\n",
            get_proc_code_size(udata.u_ptab), get_proc_code_size_blocks(udata.u_ptab),
            get_proc_data_size(udata.u_ptab), get_proc_data_size_blocks(udata.u_ptab),
            udata.u_isp - DATABASE);
        for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
        {
            const struct mapentry* b = &allocation_map[i];
            if (b->block != 0xff)
                kprintf("#%d: slot %d %s block %d %p\n",
                        i, b->slot,
                        (b->block & 0x80) ? "code" : "data",
                        b->block,
                        get_address(i));
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
                kprintf("free block %x #%d\n", i);
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

    int code_blocks = get_proc_code_size_blocks(p);
    int data_blocks = get_proc_data_size_blocks(p);
    int slot = get_slot(p);
    #ifdef DEBUG
        kprintf("alloc %d, %d code blocks, %d data blocks\n",
            get_slot(p), code_blocks, data_blocks);
        debug_blocks();
    #endif

    for (int i=0; i<code_blocks; i++)
    {
        struct mapentry* b = find_free_block(p);
        if (!b)
        {
            kprintf("pagemap_alloc: out of code memory\n");
            return ENOMEM;
        }
        b->slot = slot;
        b->block = i | 0x80;
    }

    for (int i=0; i<data_blocks; i++)
    {
        struct mapentry* b = find_free_block(p);
        if (!b)
        {
            kprintf("pagemap_alloc: out of data memory\n");
            return ENOMEM;
        }
        b->slot = slot;
        b->block = i;
    }

    p->p_page = 1;
    #ifdef DEBUG
        kprintf("done alloc, new map follows\n");
        debug_blocks();
    #endif
    return 0;
}

static int resize_blocks(
        ptptr p, int oldblocks, int newsize, int kind)
{
    int slot = get_slot(p);
    int blocks = (int)alignup(newsize, BLOCKSIZE) / BLOCKSIZE;
    if (blocks < oldblocks)
    {
        #ifdef DEBUG
            kprintf("shrinking process %s from %d to %d\n",
                kind ? "code" : "data", oldblocks, blocks);
        #endif
        for (int i=blocks; i<oldblocks; i++)
        {
            struct mapentry* b = find_block(slot, kind | i);
            b->slot = b->block = 0xff;
        }
    }
    else if (blocks > oldblocks)
    {
        #ifdef DEBUG
            kprintf("growing process %s from %d to %d\n",
                kind ? "code" : "data", oldblocks, blocks);
        #endif
        for (int i=oldblocks; i<blocks; i++)
        {
            struct mapentry* b = find_free_block(p);
            if (!b)
                return ENOMEM;
            b->slot = slot;
            b->block = kind | i;
        }
    }

    return 0;
}

/* size does *not* include udata */
int pagemap_realloc_code_and_data(usize_t codesize, usize_t datasize)
{
    struct p_tab* p = udata.u_ptab;

    codesize = (usize_t)alignup(codesize, BLOCKSIZE);
    datasize = (usize_t)alignup(datasize, BLOCKSIZE);

    /* As the code segment can never change size, we only worry about the data
     * segment here. */

    int r = resize_blocks(p, get_proc_code_size_blocks(p), codesize, 0x80);
    if (r)
        return r;
    udata.u_texttop = p->p_texttop = CODEBASE + codesize;

    r = resize_blocks(p, get_proc_data_size_blocks(p), datasize, 0x00);
    if (r)
        return r;
    udata.u_top = p->p_top = DATABASE + datasize;

    #ifdef DEBUG
        debug_blocks();
    #endif
    contextswitch(p);
    return 0;
}

usize_t pagemap_mem_used(void)
{
    usize_t count = 0;
    for (int i=0; i<NUM_ALLOCATION_BLOCKS; i++)
    {
        struct mapentry* b = &allocation_map[i];
        if (b->slot != 0xff)
            count++;
    }
    return (count*BLOCKSIZE)/1024;
}

void pagemap_init(void)
{
    #ifdef DEBUG
        kprintf("%d code blocks, %d data blocks, %d extra blocks, %d total\n",
            NUM_CODE_ALLOCATION_BLOCKS,
            NUM_DATA_ALLOCATION_BLOCKS,
            NUM_EXTRA_ALLOCATION_BLOCKS,
            NUM_ALLOCATION_BLOCKS);
    #endif
    memset(allocation_map, 0xff, sizeof(allocation_map));
    udata.u_ptab = NULL;
    udata.u_texttop = CODEBASE;
}

static void rearrange_blocks(int slot, int kind, int count)
{
    for (int i=0; i<count; i++)
    {
        int block = i + (kind ? NUM_DATA_ALLOCATION_BLOCKS : 0);
        struct mapentry* b1 = &allocation_map[block];
        int i1 = b1 - allocation_map;
        void* p1 = (void*)get_address(i1);
        if ((b1->slot != slot) || (b1->block != (kind|i)))
        {
            struct mapentry* b2 = find_block(slot, kind|i);
            if (!b2)
                panic("missing block");
            int i2 = b2 - allocation_map;
            void* p2 = (void*)get_address(i2);
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
}

void contextswitch(ptptr p)
{
    #ifdef DEBUG
        kprintf("context switch from %d to %d\n", get_slot(udata.u_ptab), get_slot(p));
    #endif

    if (!p->p_page)
        swapin(p, p->p_page2);

    int slot = get_slot(p);
    rearrange_blocks(slot, 0x80, get_proc_code_size_blocks(p));
    rearrange_blocks(slot, 0x00, get_proc_data_size_blocks(p));

    #ifdef DEBUG
        debug_blocks();
    #endif
}

static void clone_blocks(int srcslot, int destslot, int kind, int blocks)
{
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b1 = find_block(srcslot, kind|i);
        struct mapentry* b2 = find_block(destslot, kind|i);
        if (!b1 || !b2)
            panic("missing block");
        int i1 = b1 - allocation_map;
        int i2 = b2 - allocation_map;
        void* p1 = (void*)get_address(i1);
        void* p2 = (void*)get_address(i2);
        #ifdef DEBUG
            kprintf("copy #%d to #%d (%p to %p)\n", i1, i2, p1, p2);
        #endif
        memcpy(p2, p1, BLOCKSIZE);

        struct mapentry t = *b1;
        *b1 = *b2;
        *b2 = t;
    }
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
    clone_blocks(srcslot, destslot, 0x80, get_proc_code_size_blocks(p));
    clone_blocks(srcslot, destslot, 0x00, get_proc_data_size_blocks(p));
    #ifdef DEBUG
        kprintf("end clone\n");
    #endif
}

uint_fast8_t plt_canswapon(uint16_t devno)
{
    /* Only allow swapping to hd devices. */
    return (devno >> 8) == 0;
}

int swapout(ptptr p)
{
#ifdef DEBUG
    kprintf("swapping out %d (%d)\n", get_slot(p), p->p_pid);
#endif

    uint16_t page = p->p_page;
    if (!page)
        panic(PANIC_ALREADYSWAP);
    if (SWAPDEV == 0xffff)
        return ENOMEM;

    /* Are we out of swap ? */
    int16_t map = swapmap_alloc();
    if (map == -1)
        return ENOMEM;

    uint16_t swaparea = map * SWAP_SIZE;

    int slot = get_slot(p);
    int blocks = get_proc_data_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_block(slot, i);
        int blockindex = b - allocation_map;

        swapwrite(
            SWAPDEV,
            swaparea + i*(BLOCKSIZE>>BLKSHIFT),
            BLOCKSIZE,
            get_address(blockindex),
            1);

        b->slot = b->block = 0xff;
    }

    blocks = get_proc_code_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_block(slot, i | 0x80);
        int blockindex = b - allocation_map;

        swapwrite(
            SWAPDEV,
            swaparea +
                NUM_DATA_ALLOCATION_BLOCKS*(BLOCKSIZE>>BLKSHIFT) +
                i*(BLOCKSIZE>>BLKSHIFT),
            BLOCKSIZE,
            get_address(blockindex),
            1);

        b->slot = b->block = 0xff;
    }

    p->p_page = 0;
    p->p_page2 = map;
    return 0;
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */
void swapin(ptptr p, uint16_t map)
{
#ifdef DEBUG
    kprintf("swapping in %d (%d)\n", get_slot(p), p->p_pid);
#endif

    uint16_t swaparea = map * SWAP_SIZE;

    int slot = get_slot(p);
    int blocks = get_proc_data_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_free_block(p);
        int blockindex = b - allocation_map;

        swapread(
            SWAPDEV,
            swaparea + i*(BLOCKSIZE>>BLKSHIFT),
            BLOCKSIZE,
            get_address(blockindex),
            1);

        b->slot = slot;
        b->block = i;
    }

    blocks = get_proc_code_size_blocks(p);
    for (int i=0; i<blocks; i++)
    {
        struct mapentry* b = find_free_block(p);
        int blockindex = b - allocation_map;

        swapread(
            SWAPDEV,
            swaparea +
                NUM_DATA_ALLOCATION_BLOCKS*(BLOCKSIZE>>BLKSHIFT) +
                i*(BLOCKSIZE>>BLKSHIFT),
            BLOCKSIZE,
            get_address(blockindex),
            1);

        b->slot = slot;
        b->block = i | 0x80;
    }

    p->p_page = 1;
    p->p_page2 = 0;
}

arg_t brk_extend(uaddr_t addr)
{
    if (addr < (uaddr_t)udata.u_isp)
        return EINVAL;

    /* Claim more memory for this process. */
    if (pagemap_realloc_code_and_data(udata.u_texttop - CODEBASE, addr - DATABASE))
    {
        kprintf("%d: out of memory by %d\n", udata.u_ptab->p_pid,
            addr - DATATOP);
        return ENOMEM;
    }
    return 0;
}

// vim: ts=4 sw=4 et

