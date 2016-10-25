#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <buddy.h>

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

uint16_t irqstack[128];	/* Used for swapping only */
