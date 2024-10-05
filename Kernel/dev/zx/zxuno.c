#include <kernel.h>
#include <printf.h>
#include <zxuno.h>

extern uint8_t fuller, kempston, kmouse, kempston_mbmask;

/*
 *	Support routines for the ZX Uno. Can be built into _DISCARD.
 *
 *	Turn on all the I/O devices and mark them present
 *	Set the CPU to 14Mhz
 *	Turn off all the contention we can
 *	Check for 50 v 60Hz (only 50 works right now)
 *	Turn on the video and MMU modes
 *
 *	We don't do anything with the MMU and video.
 */

__sfr __banked __at 0xfc3b uno_ctrl;
__sfr __banked __at 0xfd3b uno_data;

__sfr __banked __at 0xbf3b ula_ctrl;
__sfr __banked __at 0xff3b ula_data;

uint8_t probe_zxuno(void)
{
	uint8_t n;
	uint8_t c;
	uno_ctrl = 0xff;
	uno_data = 0x00;

	/* Check if Uno is present */
	for (n = 0; n < 64; n++) {
		c = uno_data;
		if (c == 0)
			return 1;
		if (c < 32 || c > 127)
			return 0;
	}
	return 0;
}

static const uint8_t palette[] = {
	/* Normal */
	0x00,
	0x02,
	0x10,
	0x12,
	0x80,
	0x82,
	0x90,
	0x92,
	0x00,
	0x02,
	0x10,
	0x12,
	0x80,
	0x82,
	0x90,
	0x92,
	/* Bright */
	0x00,
	0x03,
	0x1C,
	0x1F,
	0xE0,
	0xE3,
	0xEC,
	0xFF,
	0x00,
	0x03,
	0x1C,
	0x1F,
	0xE0,
	0xE3,
	0xEC,
	0xFF
};

/* Turn on all the I/O devices, and fire up the turbos */
void configure_zxuno(void)
{
	uint8_t c;
	uint8_t d;
	uint8_t *p;

	uno_ctrl = 0xFF;
	uno_data = 0x00;

	kputs("ZX Uno Detected\n");
	for (c = 0; c < 64; c++) {
		d = uno_data;
		if (d == 0) break;
		kputchar(d);
	}
	kputchar('\n');
	
	uno_ctrl = 0;
	if (uno_data & 0x80)
		kputs("Warning: ZX Uno configuration is locked.\n");
	else {
		c = uno_data;
		/* Contention off, Pentagon timing (as best), NMI off,
		   DIVMMC on */
		c &= ~0x10;	
		c |= 0x66;
		uno_data = c;
	}
	uno_ctrl = 6;
	uno_data = 0x12;	/* Plug in stick is Kempston, alt is fuller */
	kempston = 1;
	fuller = 1;
	kmouse = 1;

	/* Vroooomm.... */	
	uno_ctrl = 0x0B;
	c = uno_data;
	c &= 0x3F;
	c |= 0x80;		/* 14MHz (don't set C0!!!) */
	uno_data = c;

	uno_ctrl = 0x0E;
	c = uno_data;
	c &= ~0x80;		/* SD on */
	c |= 0x40;		/* Horizontal MMU and Timex video on */
	uno_data = c;

	uno_ctrl = 0x0F;
	c = uno_data;
	c &= 0xF8;		/* ULAplus, Radistano, Timex on */
	uno_data = c;

	/* FIXME: do we need to set radasctrl |= 3 */
	uno_ctrl = 0xFB;
	if (uno_data & 1) 	/* 60 HZ */
		kputs("Warning 60Hz not yet supported.\n");

	p = palette;
	/* Initial palette */
	for (c = 0; c < 32; c++) {
		ula_ctrl = c;
		ula_data = *p;
		ula_ctrl = c + 32;
		ula_data = *p++;
	}
}

uint8_t locked_zxuno(void)
{
	uno_ctrl = 0;
	return uno_data & 0x80;
}
