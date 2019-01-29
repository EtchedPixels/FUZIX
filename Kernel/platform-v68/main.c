#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

void platform_idle(void)
{
	/* FIXME: disable IRQ, run tty interrupt, re-enable ? */
}

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

uaddr_t pagemap_base(void)
{
	return 0x20000UL;
}

uint8_t platform_param(char *p)
{
	return 0;
}

void platform_discard(void)
{
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

arg_t _memalloc(void)
{
	udata.u_error = ENOSYS;
	return -1;
}

arg_t _memfree(void)
{
	udata.u_error = ENOSYS;
	return -1;
}

/* Live udata and kernel stack */
u_block udata_block;
uint16_t irqstack[128];	/* Used for swapping only */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x40);
}

