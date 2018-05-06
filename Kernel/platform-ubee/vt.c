#include <kernel.h>
#include <devtty.h>
#include <ubee.h>
#include <vt.h>

/*
 *	We use a variant of the generic code for now but we need a different
 *	helper as we bank the video memory. We should also start using hard
 *	cursors as the 6545 has one
 */

static void char_addr(unsigned int y1, unsigned char x1)
{
	vtaddr = VT_WIDTH * y1 + x1;
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
        char_addr(y,x);
        vtcount = 1;
        vtchar = c;
        vwrite();
}

void clear_lines(int8_t y, int8_t ct)
{
	char_addr(y, 0);
	vtcount = ct * (uint16_t)VT_WIDTH;
	vtchar = ' ';
	vwrite();
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	char_addr(y, x);
	vtcount = l;
	vtchar = ' ';
	vwrite();
}

void cursor_on(int8_t y, int8_t x)
{
	char_addr(y,x);
	do_cursor_on();
}

void vtattr_notify(void)
{
	if (ubee_model == UBEE_256TC) {
		vtattrib = (((uint16_t)vtink) << 8) | (((uint16_t)vtpaper) << 12);
		if (vtattr & VTA_FLASH)
			vtattrib |= 0x80;
		if (vtattr & VTA_INVERSE)
			vtattrib |= 0x40;
		/* 5 4 must be zero 3-0 are extended font select -> 0 for now */
	} else if (ubee_model == UBEE_PREMIUM || ubee_model == UBEE_BASIC) {
		if (vtattr & VTA_INVERSE)
			vtattrib = (((uint16_t)vtpaper) << 8) | (((uint16_t)vtink) << 12);
		else
			vtattrib = (((uint16_t)vtink) << 8) | (((uint16_t)vtpaper) << 12);
		/* 7-4 MBZ, 3-0 are extended font select */
	} else
		vtattrib = 0;
}

