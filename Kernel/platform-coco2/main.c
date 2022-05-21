#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <carts.h>
#include <blkdev.h>

uint8_t membanks;
uint8_t system_id;
uint8_t cartslots = 1;
uint8_t carttype[4];
uint8_t bootslot = 0;
uint16_t swap_dev;

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

/* Find a cartridge or it's slot */
int cart_find(int id)
{
	int i;
	for (i = 0; i < id; i++) {
		if (carttype[i] == id)
			return i;
	}
	return -1;
}

unsigned char vt_mangle_6847(unsigned char c)
{
	if (c >= 96)
		c -= 32;
	c &= 0x3F;
	return c;
}
