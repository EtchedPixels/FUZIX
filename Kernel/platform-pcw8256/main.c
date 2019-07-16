#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = 0xC000;

/* Kernel is 0-3 screen for now is 4 and bits of 5
   Apps 6,7,8,9,10,11,12,13,14,15 etc
   
   This *may* change once we sort the memory map out sanely so that we are
   only using 4 for kernel/screen. More likely though we will instead use
   the split bank for buffers ? */

void map_init(void)
{
	udata.u_ptab->p_page = 0x8686;
	udata.u_ptab->p_page2 = 0x8686;
}

void pagemap_init(void)
{
	int i;
	/* FIXME: don't hardcode for 512K machine */
	for (i = 0x87; i < 0xA0; i++)
		pagemap_add(i);
}

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
