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
#if defined CONFIG_USIFAC_SERIAL
static char tbuf2[TTYSIZ];
#endif

uint8_t vtattr_cap = VTA_UNDERLINE;
extern uint8_t curattr;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
#if defined CONFIG_USIFAC_SERIAL
	_CSYS | CBAUD	
#endif
};


struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
#if defined CONFIG_USIFAC_SERIAL
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
#endif
};
#if defined CONFIG_USIFAC_SERIAL
static const uint8_t baudtable[] = { /*Usifac baudrate commands*/

	0xff,			/* 50 */
	0xff,			/* 75 */
	0xff,			/* 110 */
	0xff,			/* 134.5 */
	0xff,			/* 150 */
	10,			/* 300 */
	0xff,			/* 600 */
	0xff,			/* 1200 */
	11,			/* 2400 */
	0xff,			/* 4800 */
	12,			/* 9600 */
	13,			/* 19200 */
	14,			/* 38400 */
	15,			/* 57600 */
	16,			/* 115200 */
};
#endif
/* tty1 is the screen */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n'){
		tty_putc(1, '\r');
#ifndef CONFIG_USIFAC_SLIP
		tty_putc(2, '\r');
#endif
	}
	tty_putc(1, c);
#ifndef CONFIG_USIFAC_SLIP
	tty_putc(2, c);
#endif
}

/* Both console and debug port are always ready */
ttyready_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	switch (minor){
		case 1:
			vtoutput(&c, 1);
			break;
#if defined CONFIG_USIFAC_SERIAL			
		case 2:
			usifdata = c;
			break;
#endif
	}
}

int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_setup(uint8_t minor, uint8_t flags)
{
#if defined CONFIG_USIFAC_SERIAL
	struct termios *t = &ttydata[minor].termios;
	uint16_t cflag = t->c_cflag;
	uint8_t baud;
	used(flags);

	if (minor == 2){
		baud = cflag & CBAUD;
		if (baudtable[baud] == 0xFF){
			usifctrl = USIFAC_SET_115200B_COMMAND; /*Default 115200 for not supported baudrate request*/
			cflag &= ~CBAUD;
			cflag |= B115200;
			t->c_cflag = cflag;
		}
		else usifctrl = baudtable[baud];
	}
#endif
}
#if defined CONFIG_USIFAC_SERIAL
void tty_pollirq_usifac(void)
{		
	while (usifctrl == 0xff)
		tty_inproc(2, usifdata);
	tty_outproc(2);
}
#endif
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


int cpctty_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t c;
	switch (minor){
		case 1:
			switch(arg){
				case VTBORDER:
					c = ugetc(ptr);
					gatearray = PENR_BORDER_SELECT;
					vtborder &= INKR_COLOR_SET;
					vtborder |= c & 0x1F;
					gatearray = vtborder;
					return 0;
				case VTINK:
					c = ugetc(ptr);
					gatearray = PENR_INK_SELECT;
					vtink &= INKR_COLOR_SET;
					vtink |= c & 0x1F;
					gatearray = vtink;
					return 0;
				case VTPAPER:
					c = ugetc(ptr);
					gatearray = PENR_PAPER_SELECT;
					vtpaper &= INKR_COLOR_SET;
					vtpaper |= c & 0x1F;
					gatearray = vtpaper;
					return 0;
				default:
					return vt_ioctl(minor, arg, ptr);
				}
			break;
#if defined CONFIG_USIFAC_SERIAL
		default:
			return tty_ioctl(minor, arg, ptr);
#endif
		}
			
}
