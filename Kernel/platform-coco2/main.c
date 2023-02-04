#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

uint8_t system_id;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}

void do_beep(void)
{
}

void plt_discard(void)
{
}

unsigned char vt_mangle_6847(unsigned char c)
{
	if (c >= 96)
		c -= 32;
	c &= 0x3F;
	return c;
}
