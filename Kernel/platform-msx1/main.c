#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

/* These are set by the msx startup asm code */
uint16_t vdpport = 0x99 + 256 * 40;
uint16_t infobits;

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
 *	We have one bank per 32K and we number them upwards from 1 as the
 *	core kernel code uses 0 for swapped out.
 */

void pagemap_init(void)
{
    int i = procmem / 32;
    while (i > 0)
        pagemap_add(i--);
}

void map_init(void)
{
    if (procmem == 0)
	panic("MegaRAM not found.\n");
}


void platform_interrupt(void)
{
	timer_interrupt();
}
