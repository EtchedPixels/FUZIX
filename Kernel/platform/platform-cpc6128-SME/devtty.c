#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <keycode.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>
#include <input.h>
#include <devinput.h>

static char tbuf1[TTYSIZ];

uint8_t vtattr_cap = VTA_UNDERLINE;
extern uint8_t curattr;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};


struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* tty1 is the screen */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(0, '\r');
	tty_putc(0, c);
}

/* Both console and debug port are always ready */
ttyready_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
	vtoutput(&c, 1);
}

int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

void tty_data_consumed(uint8_t minor)
{
}


/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

//Inherited from spectrum zx+3. For now ignore attributes, try later to implement inverse and underline touching char output directly
void vtattr_notify(void)
{
	// Attribute byte fixups: not hard as the colours map directly
	//   to the spectrum ones 
/*	if (vtattr & VTA_INVERSE)
		curattr =  ((vtink & 7) << 3) | (vtpaper & 7);
	else
		curattr = (vtink & 7) | ((vtpaper & 7) << 3);

	if (vtattr & VTA_FLASH)
		curattr |= 0x80;
	// How to map the bright bit - we go by either
	if ((vtink | vtpaper) & 0x10)
		curattr |= 0x40;
*/
	//we are now debugging other things
	//vtink = 26;
	//vtpaper = 1;
	//cpcvt_ioctl(1, VTINK, &vtink);
	//cpcvt_ioctl(1, VTPAPER, &vtpaper);

}


__sfr __banked __at 0x7F00 gatearray;
/* see: https://www.cpcwiki.eu/index.php/Gate_Array */

int cpcvt_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t c;
	if (minor == 1 && arg == VTBORDER) {
		c = ugetc(ptr);
		gatearray = PENR_BORDER_SELECT;
		vtborder &= INKR_COLOR_SET;
		vtborder |= c & 0x1F;
		gatearray = vtborder;
		return 0;
	}
		if (minor == 1 && arg == VTINK) {
		c = ugetc(ptr);
		gatearray = PENR_INK_SELECT;
		vtink &= INKR_COLOR_SET;
		vtink |= c & 0x1F;
		gatearray = vtink;
		return 0;
	}
		if (minor == 1 && arg == VTPAPER) {
		c = ugetc(ptr);
		gatearray = PENR_PAPER_SELECT;
		vtpaper &= INKR_COLOR_SET;
		vtpaper |= c & 0x1F;
		gatearray = vtpaper;
		return 0;
	}
	return vt_ioctl(minor, arg, ptr);
}
