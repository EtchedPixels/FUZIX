#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>


struct blkbuf *bufpool_end = bufpool + NBUFS;

void plt_discard(void)
{
	extern uint8_t discard_size;
	bufptr bp = bufpool_end;

	kprintf("%d buffers reclaimed from discard\n", discard_size);
	
	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}


void plt_idle(void)
{
}

void do_beep(void)
{
}

/*
 Map handling: We have flexible paging. Each map table consists
 of a set of pages with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    int i;
    /*  We have 64 8k pages for a CoCo3 so insert every other one
     *  into the kernel allocator map.
     */
    for (i = 10; i < 64; i+=2)
        pagemap_add(i);
    /* add common page last so init gets it */
    pagemap_add(6);
}

void map_init(void)
{
}


uint8_t plt_param(char *p)
{
	return 0;
}
