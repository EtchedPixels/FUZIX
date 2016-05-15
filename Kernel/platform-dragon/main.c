#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint8_t platform_param(unsigned char *p)
{
	return 0;
}

void platform_idle(void)
{
}

void do_beep(void)
{
}

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
}

void map_init(void)
{
}

unsigned char vt_mangle_6847(unsigned char c)
{
	if (c >= 96)
		c -= 32;
	c &= 0x3F;
	return c;
}
