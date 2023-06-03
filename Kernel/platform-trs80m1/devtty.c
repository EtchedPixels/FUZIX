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
#include <stdarg.h>
#include <trs80.h>

/* FIXME: move to external tty buffers and bank them */
static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

uint8_t curtty;			/* output side */
static uint8_t inputtty;	/* input side */
static struct vt_switch ttysave[2];
struct vt_repeat keyrepeat = { 40, 4 };
extern uint8_t *vtbase[2];

/* Default to having /dev/tty and the two consoles openable. Our probe
   routine will add tty3/tty4 as appropriate */

static uint8_t ports = 7;

/* The Video Genie EG3020 is similar but the TR1865 is
   data in: F8, status out F8, data out: F9 status in F9,
   baud by switches.

   Or at least it probably does. In theory you can use an adapter
   cable and Tandy bits so we treat them as two ports */

__sfr __at 0xE8 tr1865_ctrl;
static uint8_t tr1865_ctrl_save;
__sfr __at 0xE9 tr1865_baud;
__sfr __at 0xEA tr1865_status;
__sfr __at 0xEB tr1865_rxtx;

__sfr __at 0xF8 vg_tr1865_wrst;
__sfr __at 0xF9 vg_tr1865_ctrd;
static uint8_t vg_tr1865_ctrd_save;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2}
};

static uint8_t trs_flow;		/* RTS/CTS */

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS|CBAUD|CSIZE|CRTSCTS|CSTOPB,
	_CSYS|CSIZE|CRTSCTS|CSTOPB,
	_CSYS|CSIZE|CRTSCTS|CSTOPB
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
	uint8_t reg;
	if (minor < 3)
		return TTY_READY_NOW;
	/* RTS/CTS is supported by the hardware. We assume delays will be
	   short as with our rather limited serial if we go off and schedule
	   something else each flow control it will get horribly slow */
	if (minor == 3) {
		if (ttydata[3].termios.c_cflag & CRTSCTS) {
			reg = tr1865_ctrl;
			if (!(reg & 0x80))
				return TTY_READY_SOON;
		}
		reg = tr1865_status;
		return (reg & 0x40) ? TTY_READY_NOW : TTY_READY_SOON;
	}
	/* minor == 4 */
	reg = vg_tr1865_wrst;
	if (ttydata[4].termios.c_cflag & CRTSCTS) {
		/* CTS ? */
		if (!(reg & 0x40))
			return TTY_READY_SOON;
	}
	return (reg & 0x80) ? TTY_READY_NOW : TTY_READY_SOON;
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
		tr1865_rxtx = c;
	else if (minor == 4)
		vg_tr1865_wrst = c;
	else {
		if (video_mode == 2)	/* Micrografyx */
			return;
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
	if (trs_flow & (1 << minor)) {
		if (minor == 3) {
			/* We have space.. raise RTS */
			if (!fullq(&ttyinq[3]))
				tr1865_ctrl = tr1865_ctrl_save|0x01;
		}
		if (minor == 4) {
			/* We have space.. raise RTS */
			if (!fullq(&ttyinq[4]))
				vg_tr1865_ctrd = vg_tr1865_ctrd_save|0x01;
		}
	}
}

/* Only the Model III has this as an actual interrupt */
void tty_interrupt(void)
{
	uint8_t reg = tr1865_status;
	if (reg & 0x80) {
		reg = tr1865_rxtx;
		tty_inproc(3, reg);
	}
	if ((trs_flow & 8) && fullq(&ttyinq[3]))
		tr1865_ctrl = tr1865_ctrl_save & ~1;
}

void tty_poll(void)
{
	uint8_t reg;

	/* Do the VG port */
	if (ports & 0x10) {
		reg = vg_tr1865_wrst;
		if (reg & 0x01) {
			reg = vg_tr1865_ctrd;
			tty_inproc(4, reg);
		}
		if ((trs_flow & 0x10) && fullq(&ttyinq[4]))
			vg_tr1865_ctrd = vg_tr1865_ctrd_save & ~1;
	}
	/* Do the Model I/III port */
	if (ports & 0x08)
		tty_interrupt();
}

/* Called to set baud rate etc */
static const uint8_t trsbaud[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 10, 14, 15
};

static const uint8_t trssize[4] = {
	0x00, 0x40, 0x20, 0x60
};

void tty_setup(uint8_t minor, uint8_t flags)
{
	uint8_t baud;
	uint8_t ctrl = 3;		/* DTR|RTS */
	struct tty *t = ttydata + minor;

	if (minor < 3)
		return;

	if (minor != 3 || trs80_model == LNW80) {
		baud = ttydata[3].termios.c_cflag & CBAUD;
		if (baud > B19200) {
			ttydata[3].termios.c_cflag &= ~CBAUD;
			ttydata[3].termios.c_cflag |= B19200;
			baud = B19200;
		} else
			baud = trsbaud[baud];

		tr1865_baud = baud | (baud << 4);

	}
	if (t->termios.c_cflag & PARENB) {
		if (t->termios.c_cflag & PARODD)
			ctrl |= 0x80;
	} else
		ctrl |= 0x8;	/* No parity */
	ctrl |= trssize[(t->termios.c_cflag & CSIZE) >> 4];

	if (t->termios.c_cflag & CRTSCTS)
		trs_flow |= (1 << minor);
	else
		trs_flow &- ~(1 << minor);
	if (minor == 3) {
		tr1865_ctrl_save = ctrl;
		tr1865_ctrl = ctrl;
	} else {
		vg_tr1865_ctrd_save = ctrl;
		vg_tr1865_ctrd = ctrl;
	}
}

int trstty_open(uint8_t minor, uint16_t flags)
{
	/* Serial port cards are optional */
	if (minor < 8 && !(ports & (1 << minor))) {
		udata.u_error = ENODEV;
		return -1;
	}
	return tty_open(minor, flags);
}

int trstty_close(uint8_t minor)
{
	if (ttydata[minor].users == 0) {
		trs_flow &= ~(1 << minor);
		if (minor == 3)
			tr1865_ctrl = 0;	/* Drop carrier and rts */
		else if (minor == 4)
			vg_tr1865_ctrd = 0;
	}
	return tty_close(minor);
}

int tty_carrier(uint8_t minor)
{
	if (minor < 3)
		return 1;
	if (minor == 3) {
		if (tr1865_ctrl & 0x80)
			return 1;
	} else if (vg_tr1865_ctrd & 0x10)
		return 1;
	return 0;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

void trstty_probe(void)
{
	if (vg_tr1865_wrst != 0xFF)
		ports |= (1 << 4);
	if (tr1865_status != 0xFF)
		ports |= (1 << 3);
}

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
		keyin[i] = *(uint8_t *) (0x3800 | (1 << i));
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
	/* F1-F4 are only present on Video Genie II/Dick Smith II */
	{'x', 'y', 'z', 0, KEY_F2, KEY_F3, KEY_F4, KEY_F1},
	{'0', '1', '2', '3', '4', '5', '6', '7'},
	{'8', '9', ':', ';', ',', '-', '.', '/'},
	{KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, 0 /*KEY_DOWN */ , KEY_BS, KEY_DEL, ' '},
	/* The Model 1 only has bit 0 of this for its shift key. The Model 3
	   has bit 2 for right shift. Some add-ons used bit 4 for control,
	   other things borrowed the down arrow
	   The VideoGenie has MS on bit 1, RPT on 3 and CTRL on 4 */
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
