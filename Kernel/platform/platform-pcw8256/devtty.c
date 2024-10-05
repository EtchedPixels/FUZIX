#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>
#include <vt.h>
#include <input.h>
#include <devinput.h>
#include <pcw8256.h>

#undef  DEBUG            /* UNdefine to delete debug code sequences */

uint8_t vtattr_cap;
struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

__sfr __at 0xE0	dart0d;
__sfr __at 0xE1 dart0c;
__sfr __at 0xE2	dart1d;		/* Not used, or exposed by the CPS8256 */
__sfr __at 0xE3 dart1c;
__sfr __at 0xE4	ctc0;
__sfr __at 0xE5 ctc1;
__sfr __at 0xE7 ctcmode;

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];
static uint8_t tbuf3[TTYSIZ];

static uint8_t sleeping;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 }
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,	/* For now - need to do DART code */
	_CSYS	/* For now - need to do DART code */
};

/* console driver for errors etc */
void kputchar(char c)
{
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

static uint8_t dartrr(uint8_t r)
{
    uint8_t ch;
    irqflags_t irq = di();
    dart0c = r;
    ch = dart0c;
    irqrestore(irq);
    return ch;
}

static void dartwr(uint8_t r, uint8_t v)
{
    irqflags_t irq = di();

    dart0c = r;
    dart0c = v;

    irqrestore(irq);
}

/* Helpers for the CPS8256 centronics port. It borrows a couple of port B
   bits for its own purposes */

uint8_t cps_centronics_busy(void)
{
    uint8_t irq;
    uint8_t ch;

    irq = di();
    dart1c = 0;
    ch = dart1c;
    irqrestore(irq);

    return ch & 0x80;
}

static void nap(void)
{
}

void cps_centronics_strobe(uint8_t c)
{
    irqflags_t irq = di();
    dart1c = 5;
    dart1c = 0x80;
    nap();
    dart1c = 5;
    dart1c = 0x00;
    irqrestore(irq);
    used(c);
}

ttyready_t tty_writeready(uint8_t minor)
{
    if (minor == 1)
        return TTY_READY_NOW;
    else
        return (dartrr(0) & 4) ? TTY_READY_NOW : TTY_READY_SOON;
}

extern void bugout(uint16_t c);

void tty_putc(uint8_t minor, unsigned char c)
{
    if (minor == 1)
        vtoutput(&c, 1);
    else if (minor == 2)
        dart0d = c;
}

static uint8_t dcd = 0;

void tty_irq(void)
{
    uint8_t s = dartrr(0);
    uint8_t c;
    if (s & 1) {
        c = dart0d;
        tty_inproc(2, c);
    }
    if ((s & 4) && (sleeping & 4)) {
	tty_outproc(2);
	sleeping &= ~4;
    }
    if ((s & 0x08) != dcd) {
        dcd = s & 0x08;
        if (dcd)
                tty_carrier_raise(2);
	else
		tty_carrier_drop(2);
    }
}

static uint16_t dartbaud[] = {
	0x000D,	/* B0 */
	0x09C4,	/* 50 */
	0x0683,	/* 75 */
	0x0470,	/* 110 */
	0x039F,	/* 134.5 */
	0x0341,	/* 150 */
	0x01A0,	/* 300 */
	0x00D0,	/* 600 */
	0x0068,	/* 1200 */
	0x0034,	/* 2400 */
	0x001A,	/* 4800 */
	0x000D,	/* 9600 */
	0x0007,	/* 19200 */
	0,	/* No 38400 or better */
	0,
	0
};

/* Called to set baud rate etc */
/* FIXME: CTS/RTS */
void tty_setup(uint8_t minor, uint8_t flags)
{
	uint16_t cflag;
	uint8_t baud;
	uint8_t r;

	used(flags);

	if (minor == 1)
		return;
	cflag = ttydata[1].termios.c_cflag;
	baud = cflag & CBAUD;
	if (baud > B19200) {
		baud = B19200;
		ttydata[1].termios.c_cflag &= ~CBAUD;
		ttydata[1].termios.c_cflag |= B19200;
	}
	/* Set transmit rate */
	ctcmode = 0x36;
	ctc0 = dartbaud[baud] >> 8;
	ctc0 = dartbaud[baud];
	/* And receive */
	ctcmode = 0x76;
	ctc0 = dartbaud[baud] >> 8;
	ctc0 = dartbaud[baud];

	dartwr(0, 0x18);	/* Reset */
	dartwr(1, 0x1B);	//* Ext int and RX int, RX int on all */

	r = (cflag & CSIZE) << 2;/* Turn CS5-CS8 into top 2 bits of byte */
	r |= 1;			/* RX enable */
	dartwr(3, r);	/* 8bit, rx enable */

	r = 0x44;		/* x 16 clock, 1 stop */
	if (cflag & PARENB) {
		r |= 1;
		if (!(cflag & PARODD))
			r |= 2;
	}
	if (cflag & CSTOPB)
		r |= 8;
	dartwr(4, r);
	r = (cflag & CSIZE) << 1;	/* Put size bits in the right spot */
	if (baud != B0)
		r |= 0x82;		/* DTR, RTS */
	dartwr(5, r);
}

int pcwtty_close(uint_fast8_t minor)
{
	int r = tty_close(minor);
	if (minor == 2 && ttydata[2].users == 0)
		dartwr(5, 0);	/* Drop DTR/RTS */
	return r;
}

int tty_carrier(uint8_t minor)
{
	uint8_t r;
	if (minor == 1)
		return 1;
	r = dartrr(0);
	return !!(r & 0x08);
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= (1 << minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* Pending better ioctl bits set up for 9600 8N1 */



/*********************** Keyboard **************************/

uint8_t keymap[12];
static uint8_t *keyin = (uint8_t *)0xFFF0;
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[12] = {	/* Shift keys */
    0,0,0x20,0,
    0,0,0,0,
    0x40,0,0x80,0			/* Fixme: shiftlock */
};

static void keyproc(void)
{
	uint8_t i;
	uint8_t key;
	uint8_t kin;

	/* We have 12 bytes of keys to scan on the PCW, we don't need to
	   touch the extras */
	for (i = 0; i < 12; i++) {
	        kin = keyin[i];		/* MMIO so changes under us */
		key = kin ^ keymap[i];
		if (key) {
		        /* Scan for changes */
			int n;
			int m = 128;
			for (n = 7; n >= 0; n--) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						keysdown--;
						if (keyboard_grab == 3) {
							queue_input(KEYPRESS_UP);
							queue_input(keyboard[i][n]);
						}
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
				m >>= 1;
			}
		}
		keymap[i] = kin;
	}
}

const uint8_t keyboard[12][8] = {
	{'2', '3', '6', '9', KEY_PASTE ,KEY_F1, '0', KEY_F3},
	{'1', '5', '4', '8', KEY_COPY, KEY_CUT, KEY_PRINT, KEY_EXIT},
	{KEY_PLUS, KEY_HALF, 0, '7', '#', 13, ']', KEY_DELR},
	{'.', '/', ';', '$'/*FIXME*/, 'p', '[', '-', '='},
	{',', 'm', 'k', 'l', 'i', 'o', '9', '0'},
	{' ', 'n', 'j', 'h', 'y', 'u', '7', '8'},
	{'v', 'b', 'f', 'g', 't', 'r', '5', '6'},
	{'x', 'c', 'd', 's', 'w', 'e', '3', '4'},
	{'z', 0, 'a', KEY_TAB, 'q', KEY_ESC, '2', '1'},
	{KEY_BS, 0, 0, 0, 0, 0, 0, 0},
	{0, '.', 13, KEY_F7, KEY_MINUS, KEY_CANCEL, 0, KEY_F5},
	/* FIXME: What to do with ALT and EXTRA */
	{0, 0, 0, 0, 0, 0, 0, 0}
};

const uint8_t shiftkeyboard[12][8] = {
	{'2', '3', '6', '9', KEY_PASTE, KEY_F2, '0', KEY_F4},
	{'1', '5', '4', '8', KEY_COPY, KEY_CUT, KEY_PRINT, KEY_EXIT},
	{KEY_PLUS, '@', 0, '7', '>', 13, '}', KEY_DELR},
	{KEY_DOT, '?', ':', '<', 'P', '{', '_', '+'},
	{'\'', 'M', 'K', 'L', 'I', 'O', '(', ')'},
	{' ', 'N', 'J', 'H', 'Y', 'U', '&', '*'},
	{'V', 'B', 'F', 'G', 'T', 'R', '%', '^'},
	{'X', 'C', 'D', 'S', 'W', 'E', KEY_POUND, '$'},
	{'Z', 0, 'A', KEY_TAB, 'Q', KEY_STOP, '"', '!'},
	{KEY_BS, 0, 0, 0, 0, 0, 0, 0},
	{0, '.', 13, KEY_F8, KEY_MINUS, KEY_CANCEL, 0, KEY_F6},	/* What to do with ALT */
	{0, 0, 0, 0, 0, 0, 0, 0}
};

static uint8_t capslock = 0;


/* What should we do with extra, not as if you can run emacs */

static void keydecode(void)
{
	uint8_t c;
	uint8_t m = 0;

	if (keybyte == 8 && keybit == 6) {
		capslock = 1 - capslock;
		return;
	}
	if (keymap[2] & (1 << 5)) {	/* shift */
		m = KEYPRESS_SHIFT;
		c = shiftkeyboard[keybyte][7-keybit];
	} else
		c = keyboard[keybyte][7-keybit];

	if (keymap[10] & 0x80) {	/* alt */
		if (c > 31 && c < 127)
			c &= 31;
		m |= KEYPRESS_CTRL;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c) {
		switch (keyboard_grab) {
		case 0:
			vt_inproc(1, c);
			break;
		case 1:
			if (!input_match_meta(c)) {
				vt_inproc(1, c);
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

/* Vblank ticker */
uint8_t vblank;

__sfr __at 0xF4 timer;
__sfr __banked __at 0x00fd printer;

/* FIXME: keyboard repeat
          floppy motor, serial etc */
void plt_interrupt(void) 
{
	/* The low 4 bits count timer events, 0 means this interrupt
	   was from something else only */
	uint8_t t = timer & 0x0F;

	if (timer == 0) {
		/* serial or similar event, so poll them */
		tty_irq();
		printer;	/* Clear any Daisy interrupt */
		return;
	}

	newkey = 0;
	keyproc();

	while(t--) {
		if (keysdown && keysdown < 3) {
			if (newkey) {
				keydecode();
				kbd_timer = keyrepeat.first;
			} else if (! --kbd_timer) {
				keydecode();
				kbd_timer = keyrepeat.continual;
			}
		}
		vblank++;
		wakeup(&vblank);
		/* Should also run the mouse accumulator here FIXME */
		timer_interrupt();
	}
}
