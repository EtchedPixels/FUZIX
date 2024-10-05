#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <input.h>
#include <devinput.h>
#include <devgfx.h>
#include <genie.h>

/* Two consoles, may also have serial I/O */
static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

uint8_t curtty;			/* output side */
static uint8_t inputtty;	/* input side */
static struct vt_switch ttysave[2];
struct vt_repeat keyrepeat = { 40, 4 };
extern uint8_t *vtbase[2];

/* Serial card */

__sfr __at 0xD0	sioa_d;
__sfr __at 0xD1	siob_d;
__sfr __at 0xD2 sioa_c;
__sfr __at 0xD3 siob_c;
__sfr __at 0xF1 baudgen;
__sfr __at 0xF2 btx;

/* Default to having /dev/tty and the two consoles openable. Our probe
   routine will add tty3/tty4 as appropriate */

static uint8_t ports = 7;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2}
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	/* TODO: crtscts */
	_CSYS|CBAUD|CSIZE|CRTSCTS|CSTOPB,
	_CSYS|CBAUD|CSIZE|CRTSCTS|CSTOPB,
};

/* Write to system console */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	irqflags_t irq;
	uint8_t reg;
	if (minor < 3)
		return TTY_READY_NOW;
	irq = di(); 
	if (minor == 3) {
		sioa_c = 0;
		reg = sioa_c;
	} else {
		siob_c = 0;
		reg = siob_c;
	}
	irqrestore(irq);
	return (reg & 0x04) ? TTY_READY_SOON : TTY_READY_NOW;
}

static uint8_t vtbuf[64];
static uint8_t *vtq = vtbuf;

void vtflush(void)
{
	vtoutput(vtbuf, vtq - vtbuf);
	vtq = vtbuf;
}

static void vtexchange(void)
{
	/* Swap the pointers over: TRS80 video we switch by copying not
	   flipping hardware pointers */
	uint8_t *v = vtbase[0];
	vtbase[0] = vtbase[1];
	vtbase[1] = v;
	/* The cursor x/y for current tty are stale in the save area
	   so save them */
	vt_save(&ttysave[curtty]);

	/* Before we flip the memory */
	cursor_off();

	vtswap();

	/* Cursor back */
	if (!ttysave[inputtty].cursorhide)
		cursor_on(ttysave[inputtty].cursory, ttysave[inputtty].cursorx);
}

void tty_putc(uint8_t minor, unsigned char c)
{
	irqflags_t irq;

	if (minor == 3)
		sioa_d = c;
	else if (minor == 4)
		siob_d = c;
	else {
		irq = di();
		if (curtty != minor - 1) {
			/* Kill the cursor as we are changing the memory buffers. If
			   we don't do this the next cursor_off will hit the wrong
			   buffer */
			vtflush();
			vt_save(&ttysave[curtty]);
			curtty = minor - 1;
			vt_load(&ttysave[curtty]);
			/* Fix up the cursor */
		} else if (vtq == vtbuf + sizeof(vtbuf))
			vtflush();
		*vtq++ = c;
		irqrestore(irq);
	}
}

void tty_data_consumed(uint8_t minor)
{
	/* TODO */
}

static uint8_t old_ca, old_cb;

void sio_intr(void)
{
	uint8_t r = sioa_c;
	if (r & 1)
		tty_inproc(3, sioa_d);
	if (r & 2)
		sioa_c = 2 << 5;
	if ((r ^ old_ca) & 8) {
		old_ca = r;
		if (r & 8)
			tty_carrier_raise(3);
		else
			tty_carrier_lower(3);
	}
	r = siob_c;
	if (r & 1)
		tty_inproc(4, siob_d);
	if (r & 2)
		siob_c = 2 << 5;
	if ((r ^ old_cb) & 8) {
		old_cb = r;
		if (r & 8)
			tty_carrier_raise(4);
		else
			tty_carrier_lower(4);
	}
}

void tty_interrupt(void)
{
	if (siob_c & 2)
		sio_intr();
}

static uint8_t baudbits;

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
	0x05, 0xEA
};

void tty_setup(uint8_t minor, uint8_t flags)
{
	uint8_t baud;
	uint8_t ctrl = 3;		/* DTR|RTS */
	uint8_t r = 0xC4;
	struct termios *t = &ttydata[minor].termios;

	if (minor < 3)
		return;

	baud = t->c_cflag & CBAUD;
	if (baud > B19200) {
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= B19200;
		baud = B19200;
	};
	sio_r[1] = 0x01 | ((t->c_flag & CSIZE) << 2);
	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->c_cflag & CSIZE) << 1);
	if (minor == 3) {
		baudbits &= 0xF0;
		baudbits |= baudtab[baud];
	} else {
		baudbits &= 0x0F;
		baudbits |= baudtab[baud] << 4;
	}
	baudgen = baudbits;
	sio2_otir(0xAF + minor);	/* ie D2 or D3 */
}

int genietty_open(uint8_t minor, uint16_t flags)
{
	/* Serial port cards are optional */
	if (minor < 8 && !(ports & (1 << minor))) {
		udata.u_error = ENODEV;
		return -1;
	}
	return tty_open(minor, flags);
}

int genietty_close(uint8_t minor)
{
	if (ttydata[minor].users == 0) {
	}
	return tty_close(minor);
}

int tty_carrier(uint8_t minor)
{
	irqflags_t irq;
	uint8_t r;

	if (minor < 3)
		return 1;
	irq = di();
	if (minor == 3) {
		sioa_c = 0;
		r = sioa_c & 0x08;
	} else {
		siob_c = 0;
		r = siob_c & 0x08;
	}
	irqrestore(irq);
	return !!r;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

void tty_probe(void)
{
	/* TODO probe for SIO */
}

/* TODO: extra keys */
uint8_t keymap[8];
static uint8_t keyin[8];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[8] = {
	0, 0, 0, 0, 0, 0, 0, 7
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	for (i = 0; i < 8; i++) {
		/* Set one of A0 to A7, and read the byte we get back.
		   Invert that to get a mask of pressed buttons */
		keyin[i] = mmio_read(0x3800 | (1 << i));
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						if (keyboard_grab == 3) {
							queue_input(KEYPRESS_UP);
							queue_input(keyboard[i][n]);
						}
						keysdown--;
					}
				}
				if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						keysdown++;
						newkey = 1;
						keybyte = i;
						keybit = n;
					}
				}
				m += m;

			}
		}
		keymap[i] = keyin[i];
	}
}

/*
 *	The TRS80 keyboard is a little lacking in some rather useful symbols
 *
 *	There are conventions for some of these
 *	shift-0 is capslock
 *	downarrow-xx is ctrl
 *	There was also a popular keyboard mod
 *
 *	We also map as follows
 *	left arrow - backspace
 *	right arrow - delete
 *	shift left/right arrow - switch vt
 *	stop - ^C
 *	control-shift-brackets give curly brackets
 *	control-shift minus gives underscore
 *	control-shift slash gives backquote
 *	control-shift les-than gives hat
 *	control-brackets gives square brackets
 *	control-minus gives pipe
 *
 *	We may want to add others if need be
 *
 *	TODO: Where does the LNW80 hide F1/F2 ?
 */

uint8_t keyboard[8][8] = {
	{'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g'},
	{'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o'},
	{'p', 'q', 'r', 's', 't', 'u', 'v', 'w'},
	/* Genie IIs has german characters not F1-F4 TODO: unicode ? FIXME: remove FN ? */
	{'x', 'y', 'z', 0, KEY_F2, KEY_F3, KEY_F4, KEY_F1},
	{'0', '1', '2', '3', '4', '5', '6', '7'},
	{'8', '9', ':', ';', ',', '-', '.', '/'},
	{KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, 0 /*KEY_DOWN */ , KEY_BS, KEY_DEL, ' '},
	/* The VideoGenie has MS on bit 1, RPT on 3 and CTRL on 4 */
	{0, 0, 0, 0, KEY_CAPSLOCK, 0, 0, 0}
};

uint8_t shiftkeyboard[8][8] = {
	{'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G'},
	{'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O'},
	{'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W'},
	{'X', 'Y', 'Z', 0, 0, 0, 0, 0},
	{KEY_CAPSLOCK, '!', '"', '#', '$', '%', '&', '\''},
	{'(', ')', '*', '+', '<', '=', '>', '?'}	,
	{KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, 0 /*KEY_DOWN */ , KEY_LEFT, KEY_RIGHT, ' '},
	{0, 0, 0, 0, KEY_CAPSLOCK, 0, 0, 0}
};

static uint8_t capslock = 0;
static uint8_t kbd_timer;

static void keydecode(void)
{
	uint8_t c;
	uint8_t m = 0;

	/* Convention for capslock or the mod */
	/* Only the model 3 has right shift (2) */
	if (keymap[7] & 3) {	/* shift (left/right) */
		m = KEYPRESS_SHIFT;
		c = shiftkeyboard[keybyte][keybit];
		/* VT switcher */
		if (c == KEY_LEFT || c == KEY_RIGHT) {
			c -= KEY_RIGHT;
			c ^= 1;
			if (inputtty != c) {
				inputtty = c;
				vtexchange();	/* Exchange the video and backing buffer */
			}
			return;
		}
	} else
		c = keyboard[keybyte][keybit];

	if (c == KEY_CAPSLOCK) {
		capslock = 1 - capslock;
		return;
	}
	/* The keyboard lacks some rather important symbols so remap them
	   with control (down arrow) */
	if ((keymap[6] | keymap[7]) & 16) {	/* control */
		m |= KEYPRESS_CTRL;
		if (keymap[7] & 3) {	/* shift */
			if (c == '(')
				c = '{';
			if (c == ')')
				c = '}';
			if (c == '-')
				c = '_';
			if (c == '/')
				c = '``';
			if (c == '<')
				c = '^';
		} else {
			if (c == '8' /*'(' */ )
				c = '[';
			else if (c == '9' /*')' */ )
				c = ']';
			else if (c == '-')
				c = '|';
			else if (c > 31 && c < 127)
				c &= 31;
		}
	} else if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c) {
		switch (keyboard_grab) {
		case 0:
			vt_inproc(inputtty + 1, c);
			break;
		case 1:
			if (!input_match_meta(c)) {
				vt_inproc(inputtty + 1, c);
				break;
			}
			/* Fall through */
		case 2:
			queue_input(KEYPRESS_DOWN);
			queue_input(c);
			break;
		case 3:
			/* Queue an event giving the base key (unshifted)
			   and the state of shift/ctrl/alt */
			queue_input(KEYPRESS_DOWN | m);
			queue_input(keyboard[keybyte][keybit]);
			break;
		}
	}
}

/* Polled 40 times a second */
void kbd_interrupt(void)
{
	newkey = 0;
	/* TODO: fastpath check for no keys - ditto on TRS80M1 etc */
	keyproc();
	if (keysdown && keysdown < 3) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first;
		} else if (!--kbd_timer) {
			keydecode();
			kbd_timer = keyrepeat.continual;
		}
	}
	if (vtq != vtbuf)
		vtflush();
	poll_input();
}
