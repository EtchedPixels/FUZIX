#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

/* These are set by the msx startup asm code */
uint16_t vdpport = 0x99 + 256 * 80;
uint16_t infobits;
uint16_t msxmaps;

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
    int i = msxmaps - 1;
    /* Add all the RAM, except 0,1,2,3 which is the kernel data/bss, add 0
       last to become the common for init */
    while (i > 4)
        pagemap_add(i--);
    /* Init will pick this up correctly as its common */
    pagemap_add(4);
}

void map_init(void)
{
}


void platform_interrupt(void)
{
	timer_interrupt();
}
