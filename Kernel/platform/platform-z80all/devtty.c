/*
 *	TODO: KIO option
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <ps2kbd.h>

static uint8_t tbuf1[TTYSIZ];
static uint8_t sleeping;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

void kputchar(uint_fast8_t c)
{
	while(tty_writeready(1) != TTY_READY_NOW);
	if (c == '\n')
		tty_putc(1, '\r');
	while(tty_writeready(1) != TTY_READY_NOW);
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

/*
 *	Write a character to a tty. This is the normal user space path for
 *	each outbound byte. It gets called in the normal tty flow, but may
 *	also be called from an interrupt to echo characters even if the
 *	tty is busy. This one reason to implement a small transmit queue.
 *
 *	If the character echo doesn't fit just drop it. It should pretty much
 *	never occur and there is nothing else to do.
 */
void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t ch = c;
	vtoutput(&ch, 1);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

void tty_poll(void)
{
	if (in(0xF5) & 1)
		ps2kbd_byte(in(0xF4));
}

/* No talk back */
int ps2kbd_put(uint_fast8_t c)
{
	return 0;
}

uint16_t ps2kbd_get(void)
{
	return 0;
}

static void vga_set_char(uint_fast8_t ch, register uint8_t *map)
{
	/* Calculate offset and turn it around */
	register uint16_t ptr = ntohs(ch * 8 + 3072);
	register unsigned c = 0;
	while(c++ < 8) {
		out(ptr, *map++);
		ptr += 256;
	}
}

static const struct fontinfo fonti = {
	0, 128, 128, 191, FONT_INFO_8X16
};

/* Ioctl interface as we can support user graphics */
int vga_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	register uint_fast8_t i = 0;
	uint8_t map[8];
	uint_fast8_t top = 128;

	if (minor != 1)
		return tty_ioctl(minor, arg, ptr);

	switch(arg) {
	case VTFONTINFO:
		return uput(&fonti, ptr, sizeof(struct fontinfo));
	case VTSETUDG:
		i = ugetc(ptr);
		ptr++;
		top = i + 1;
	case VTSETFONT:
		while(i < top) {
			if (uget(ptr, map, 8) == -1) {
				udata.u_error = EFAULT;
				return -1;
			}
			vga_set_char(i++, map);
			ptr += 16;
		}
		return 0;
	}
	return vt_ioctl(minor, arg, ptr);
}
