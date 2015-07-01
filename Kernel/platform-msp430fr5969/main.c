#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include "externs.h"

uaddr_t ramtop;
uint8_t need_resched;
uint8_t last_interrupt;

void platform_idle(void)
{
}

void do_beep(void)
{
}

void program_vectors(uint16_t* pageptr)
{
	/* On the MSP430, with no banking, changing processes doesn't
	 * touch the interrupt vectors. Therefore we don't need to
	 * reprogram them and this is a nop. Go us. */
}
