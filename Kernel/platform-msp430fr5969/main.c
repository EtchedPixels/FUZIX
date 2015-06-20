#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>

uint16_t ramtop = PROGTOP;

void platform_idle(void)
{
}

void do_beep(void)
{
}

void map_init(void)
{
}

void program_vectors(uint16_t* pageptr)
{
	/* On the MSP430, with no banking, changing processes doesn't
	 * touch the interrupt vectors. Therefore we don't need to
	 * reprogram them and this is a nop. Go us. */
}

