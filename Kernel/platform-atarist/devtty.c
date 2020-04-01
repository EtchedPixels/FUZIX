#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

/*
 *	We have three serial links on the Atari ST. The MFP provides the serial
 *	port (tty2 for now), and a pair of ACIA chips provide keyboard/mouse/js
 *	and midi. We expose the midi as a tty but not the keyboard port.
 */
#define IKBD_STATUS 0xFFFC00
#define IKBD_DATA   0xFFFC02
#define MIDI_STATUS 0xFFFC04
#define MIDI_DATA   0xFFFC06

#define MFP_TSR     0xFFFA2D
#define MFP_DATA    0xFFFA2F
#define MFP_TCDCR   0xFFFA1D
#define MFP_TDDR    0xFFFA25

#undef  DEBUG			/* UNdefine to delete debug code sequences */

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	/* FIXME CTS/RTS, CSTOPB ? */
	CSIZE|CBAUD|PARENB|PARODD|_CSYS,
	_CSYS
};

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];
static unsigned char tbuf3[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2}
};

uint8_t vtattr_cap;

/* tty1 is the screen tty2 is the serial port tty3 is MIDI */

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	*(volatile uint8_t *)0xFFFA2F = c;
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c;
	if (minor == 1)
		return TTY_READY_NOW;
	if (minor == 2) {
		/* Double check which way around */
		c = *(volatile uint8_t *)MFP_TSR;
		return (c & 0x80) ? TTY_READY_NOW : TTY_READY_SOON;
	}
	c = *(volatile uint8_t *)MIDI_STATUS;
	if (c & 0x02)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 1) {
		vtoutput(&c, 1);
	} else if (minor == 2)
		*(volatile uint8_t *)MFP_DATA = c;
	else
		*(volatile uint8_t *)MIDI_DATA = c;
}

static const uint16_t mfp_baud[] = {
	0x0104,	/* B0 */
	0x0260,	/* B50 */
	0x0240,	/* B75 */
	0x01AF, /* B110 */
	0x018F,	/* B134 */
	0x0180,	/* B150 */
	0x0140,	/* B300 */
	0x0120,	/* B600 */
	0x0110,	/* B1200 */
	0x0108,	/* B2400 */
	0x0104,	/* B4800 */
	0x0102,	/* B9600 */
	0x0101	/* B19200 */
	/* Hardware cannot support 38400 or higher */
};

void tty_setup(uint8_t minor, uint8_t flags)
{
	if (minor == 1)
		return;
	/* Mostly TOOD */
	if (minor == 3) {
		return;
	}
	if (minor == 2) {
		uint8_t speed = ttydata[2].termios.c_cflag & CBAUD;
		if (speed > B19200) {
			speed = B19200;
			ttydata[2].termios.c_cflag &= ~CBAUD;
			ttydata[2].termios.c_cflag |= B19200;
		}
		*(volatile uint8_t *)MFP_TCDCR &= 0xF0;
		*(volatile uint8_t *)MFP_TDDR = mfp_baud[speed];
		*(volatile uint8_t *)MFP_TCDCR |= mfp_baud[speed] >> 8;
	}
}

int tty_carrier(uint8_t minor)
{
	/* The serial DCD is status bit 5 but not wired */
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	/* For now.. probably worth using tx ints */
}

void tty_data_consumed(uint8_t minor)
{
}

void ser_cts(void)
{
}

void ser_dcd(void)
{
}

void ser_ri(void)
{
}

struct vt_repeat keyrepeat;

uint8_t keymap[16];
static uint8_t lastkey;
static int keysdown = 0;
static uint8_t capslock;

uint8_t keyboard[1][128] = { {
	0, KEY_ESC,
	'1','2','3','4','5','6','7','8','9','0',
	'-','=', KEY_BS, KEY_TAB,
	'q','w','e','r','t','y','u','i','o','p',
	'[',']',KEY_ENTER,
	0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
	0,'\\','z','x','c','v','b','n','m',',','.','/',0,
	0,0,' ',KEY_CAPSLOCK,
	KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,
	0,0,
	KEY_HOME,KEY_UP,0,'-',KEY_LEFT,0,KEY_RIGHT,'+',0,KEY_DOWN,
	0, KEY_INSERT,KEY_DEL,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,/* ISO ?? */
	KEY_UNDO, KEY_HELP,
	'(','/','*',')',	/* FIXME */
	'7','8','9',
	'4','5','6',
	'1','2','3',
	'0','.', KEY_ENTER,
} };

uint8_t shiftkeyboard[1][128] = { {
	0, KEY_ESC,
	'!','@','#','$','%','^','&','*','(',')',
	'~','+', KEY_BS, KEY_TAB,
	'Q','W','R','R','T','Y','U','I','O','P',
	'{','}',KEY_ENTER,
	0, 'A','S','D','F','G','H','J','L','L',':','"','~',
	0,'|','Z','X','C','V','B','N','M','<','>','?',0,
	0,0,' ',KEY_CAPSLOCK,
	KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,
	0,0,
	KEY_HOME,KEY_UP,0,'-',KEY_LEFT,0,KEY_RIGHT,'+',0,KEY_DOWN,
	0, KEY_INSERT,KEY_DEL,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,/* ISO ?? */
	KEY_UNDO, KEY_HELP,
	'(','/','*',')',	/* FIXME */
	'7','8','9',
	'4','5','6',
	'1','2','3',
	'0','.', KEY_ENTER,
} };

static void keydecode(void)
{
	uint8_t c = lastkey;

	if (c == 0x3A) {
		capslock ^= 1;
		return;
	}

	/* FIXME: wire up input layer */
	if ((keymap[5] & 0x04) || (keymap[6] & 0x40))	/* shifts */
		c = shiftkeyboard[0][c];
	else
		c = keyboard[0][c];

	if (capslock) {	/* Caps lock */
		if (c >= 'a' && c <= 'z')
			c -= 32;
	}
	if (keymap[3] & 0x20)	/* Control */
		c &= 31;
	/* What should we do with alt (0x38) */
	/* FIXME: unicode */
	vt_inproc(1, c);
}

/* It's not really a grid but we need to expose it as that */
/* FIXME: direct 37/5e/5f/59-59 to mouse */

static void keybop(uint8_t c)
{
	uint8_t i = (c & 0x7F) >> 3;
	uint8_t b = c & 7;
	uint8_t nonshift = 1;

	if (c == 0x2A || c ==0x36 || c == 0x14 || c== 0x38 || c == 0x3A)
		nonshift = 0;

	if (c & 0x80) {
		keymap[i] &= ~ (1 << b);
		keysdown -= nonshift;
		if (c == (lastkey | 0x80))
			lastkey = 0;
		return;
	}
	keymap[i] |= (1 << b);
	lastkey = c;
	keysdown += nonshift;
	/* FIXME: autorepeat */
	/* And remember not to autorepeat capslock! */
	if (keysdown && lastkey && keysdown < 3)
		keydecode();
}

static uint8_t ikbd_buf[8];
static uint8_t ikbd_ct;
static uint8_t ikbd_len = 1;
static uint8_t ikbd_busy;

static const uint8_t msg_len[10] = {
	8, 6, 3, 3, 3, 3, 7, 3, 2, 2
};

static void ikbd_message(void)
{
	uint8_t c = ikbd_buf[0];
	if (ikbd_ct == 1) {
		if (c < 0xF6) {
			ikbd_ct = 0;
			ikbd_len = 1;
			keybop(c);
		} else {
			ikbd_len = msg_len[c - 0xF6];
			return;
		}
	}
	/* A message completed */
	ikbd_ct = 0;
	ikbd_len = 1;
	switch(c) {
	case 0xF6:	/* Query reply */
		ikbd_busy = 0;
		wakeup(&ikbd_busy);
		return;
	case 0xF7:	/* Abs mouse */
		return;
	case 0xF8:	/* Rel mouse (used) */
	case 0xF9:
	case 0xFA:
	case 0xFB:
		return;
	case 0xFC:	/* rtc */
		ikbd_busy = 0;
		wakeup(&ikbd_busy);
		/*
		memcpy(ikbd_rtc, ikbd_buf + 1, 6);
		ikbd_rtc_ready = 1;
		wakeup(&ikbd_rtc_ready); */
		return;
	case 0xFD:	/* sticks both */
	case 0xFE:	/* stick 0 */
	case 0xFF:	/* stick 1 */
		return;
	}
}

static void acia_key_event(uint8_t c)
{
	ikbd_buf[ikbd_ct++] = c;
	if (ikbd_ct == ikbd_len)
		ikbd_message();
}

/* Someting happened on one of our ACIA chips - or both. We poll them on
   the transmit side for now */
void acia_interrupt(void)
{
	uint8_t st = *(volatile uint8_t *)IKBD_STATUS;
	if (st & 0x80)
		acia_key_event(*(volatile uint8_t *)IKBD_DATA);
	st = *(uint8_t *)MIDI_STATUS;
	if (st & 0x80)
		tty_inproc(3, *(volatile uint8_t *)MIDI_DATA);
}

/*
 *	Send a message to the keyboard controller
 *
 *	FIXME: timeouts
 */
void ikbd_send(uint8_t *msg, uint8_t len)
{
	irqflags_t irq = di();
	while(1) {
		if (ikbd_busy == 0) {
			ikbd_busy = 1;
			while(len) {
				while(*(volatile uint8_t *)IKBD_STATUS & 2);
				*(volatile uint8_t *)IKBD_DATA = *msg++;
			}
			irqrestore(irq);
			return;
		}
		psleep_nosig(&ikbd_busy);
		di();
	}
}

/* This is used by the vt asm code, but needs to live at the top of the kernel */
uint16_t cursorpos;


/*
 *	Graphics and vblank
 */

uint32_t vblankct;
