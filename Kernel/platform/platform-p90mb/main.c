#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

uint16_t swap_dev = 0xFFFF;

void do_beep(void)
{
}

/*
 *	MMU initialize
 */

void map_init(void)
{
}

uaddr_t ramtop;
uint8_t need_resched;

uint8_t plt_param(char *p)
{
	return 0;
}

void plt_discard(void)
{
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

void pagemap_init(void)
{
	/* Linker provided end of kernel */
	/* TODO: create a discard area at the end of the image and start
	   there */
	extern uint8_t _end;
	uint32_t e = (uint32_t)&_end;
	/* Memory runs 0 to 80000 then keeps repeating */
	kmemaddblk((void *)e, 0x80000 - e);
	/* Fix up the CPU detect - should be moved to core code post 0.4 */
	sysinfo.cpu[0] = 10;
}

/* Udata and kernel stacks */
u_block udata_block[PTABSIZE];

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x20);
}

uint8_t plt_udata_set(ptptr p)
{
	u_block *up = &udata_block[p - ptab];
	p->p_udata = &up->u_d;
	return 0;
}

void plt_interrupt(void)
{
	/* Ack the interrupt */
	*((volatile uint8_t *)0x80002023) = 4;
	/* Handle it */
	timer_interrupt();
}
