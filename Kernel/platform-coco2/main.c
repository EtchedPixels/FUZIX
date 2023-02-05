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
	/* We use 0xA0 as a cursor code */
	if (c == 0xA0)
		return 0x20;
	/* Upper case letters invert */
	if (c >= 'A' && c <= 'Z')
		return c & 0x3F;
	/* Lower case letters normal */
	if (c >= 'a' && c <= 'z')
		return (c & 0x1F) | 0x40;
	/* Upper punctuation invert */
	if (c >= 0x60)
		return (c & 0x1F) | 0x20;
	/* Otherwise normal */
	return (c & 0x3F) | 0x40;
}
