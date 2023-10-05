#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ttydw.h>
#include <libc.h>

#define DISC __attribute__((section(".discard")))

uint16_t swap_dev = 0xFFFF;
struct blkbuf *bufpool_end = bufpool + NBUFS;

DISC void plt_copyright(void)
{
	kprintf("COCO3 platform Copyright (c) 2015-2018 Brett M. Gordon\n");
}

void plt_discard(void)
{
	bufptr bp = bufpool_end;

	/* Until we switch to 8K pages then this will be E000 */
	while(bp + 1 < (void *)0xC000) {
		memset(bp, 0, sizeof(struct blkbuf));
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
		bp++;
	}
	kprintf("%d buffers reclaimed from discard\n", bp - bufpool_end);
	bufpool_end = bp;
}


void plt_idle(void)
{
}

void do_beep(void)
{
}

/* Scan memory return number of 8k pages found */
DISC int scanmem(void)
{
	volatile uint8_t *mmu = (uint8_t *) 0xffa8;
	volatile uint8_t *ptr = (uint8_t *) 0x0100;
	int i;
	for (i = 0; i < 256; i += 16) {
		*mmu = i;
		*ptr = 0;
	}
	*mmu = 0;
	*ptr = 0xff;
	*mmu = 16;
	for (i = 16; i < 256 && !*ptr;) {
		i += 16;
		*mmu = i;;
	}
	*mmu = 0;
	return i;
}


/*
 Map handling: We have flexible paging. Each map table consists
 of a set of pages with the last page repeated to fill any holes.
 
 0-7: kernel (6-7 form init common as well)
 8-B: video
 C  : paged kernel buffers
 D+ : user
 */

DISC void pagemap_init(void)
{
	int i;
	int max = scanmem() - 1;	/* Pairs so need one for the upper top */

	/*  We have 64 8k pages for a CoCo3 so insert every other one
	 *  into the kernel allocator map, skipping 3f. 3f holds our constant
	 *  page and tty buffers.
	 */
	for (i = 0x0D; i < max; i += 2)
		pagemap_add(i);
	/* add common page last so init gets it */
	pagemap_add(6);
//	/* initialize swap pages */
//	for (i = 0; i < MAX_SWAPS; i++)
//		swapmap_init(i);
}

DISC void map_init(void)
{
}

DISC uint8_t plt_param(char *p)
{
	if (!strcmp(p, "NODW")) {
		dwtype = DWTYPE_NOTFOUND;
		return -1;
	}
	if (!strncmp(p, "SWAP=", 5)) {
		swap_dev = bootdevice(p + 5);
		return -1;
	}
	if (!strncmp(p, "VTMODE=", 7)) {
		set_defmode(p);
		return -1;
	}
	return 0;
}
