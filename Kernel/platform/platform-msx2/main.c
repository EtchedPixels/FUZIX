#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t msxmaps;

struct blkbuf *bufpool_end = bufpool + NBUFS;

void plt_idle(void)
{
    __asm
    halt
    __endasm;
}

uint8_t plt_param(char *p)
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
    /* Add all the RAM, except 1-4 which are the kernel data/bss, add 4
       last to become the common for init. 0 is currently free but we
       may use it later to stash fonts, console buffers, disk buffers and
       maybe bank some code */
    while (i > 4)
        pagemap_add(i--);
    /* Init will pick this up correctly as its common */
    pagemap_add(4);
}

void plt_interrupt(void)
{
    kbd_interrupt();
    timer_interrupt();
}

/*
 *	Our disk buffer cache is slotted high above user space so we just
 *	reclaim memory back for user use and nothing is needed here.
 */
void plt_discard(void)
{
}
