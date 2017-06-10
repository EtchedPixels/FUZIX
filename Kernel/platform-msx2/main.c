#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t msxmaps;

struct blkbuf *bufpool_end = bufpool + NBUFS;

void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

void do_beep(void)
{
}

/*
 * Map handling: We have flexible paging. Each map table consists of a set
 * of pages with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    int i = msxmaps - 1;
    /* Add all the RAM, except 0,1,2,3 which is the kernel data/bss, add 0
       last to become the common for init */
    while (i > 4)
        pagemap_add(i--);
    /* Init will pick this up correctly as its common */
    pagemap_add(4);
}

void platform_interrupt(void)
{
    kbd_interrupt();
    timer_interrupt();
}

/*
 *	Once we are about to load init we can throw the boot code away
 *	and convert it into disk cache. This gets us 7 or so buffer
 *	back which more than doubles our cache size !
 */
void platform_discard(void)
{
  extern uint16_t discard_size;
  bufptr bp = bufpool_end;

  discard_size /= sizeof(struct blkbuf);

  kprintf("%d buffers reclaimed from discard\n", discard_size);

  bufpool_end += discard_size;	/* Reclaim the discard space */

  memset(bp, 0, discard_size * sizeof(struct blkbuf));
  /* discard_size is in discard so it dies here */
  for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
    bp->bf_dev = NO_DEVICE;
    bp->bf_busy = BF_FREE;
  }
}
