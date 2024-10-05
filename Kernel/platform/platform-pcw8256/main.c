#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = 0xC000;
uint16_t swap_dev = 0xFFFF;
uint8_t is_joyce;
uint8_t model;

void plt_idle(void)
{
	__asm
		halt
	__endasm;
}
