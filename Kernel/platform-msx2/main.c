#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t vdpport = 0x98;

void platform_idle(void)
{
    __asm
    halt
    __endasm;
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
    int i = /*FIXME*/ 16; /* in 16K banks */
    /* Add all the RAM, except 0,1,2,3 which is the kernel data/bss, add 0
       last to become the common for init */
    while (i > 3)
        pagemap_add(i--);
    /* Init will pick this up correctly as its common */
    pagemap_add(0);
}

void map_init(void)
{
}
