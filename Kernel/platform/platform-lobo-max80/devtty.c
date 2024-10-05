#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <input.h>
#include <devinput.h>
#include <lobo.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];

struct vt_repeat keyrepeat;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 }
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	/* FIXME CTS/RTS, CSTOPB ? */
	CSIZE|CBAUD|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|PARENB|PARODD|_CSYS
};

/* Write to system console */
void kputchar(uint_fast8_t c)
{
	/* Set the device to 2 to get kernel messages dumped to serial */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    irqflags_t irq;
    uint_fast8_t r;
    /* 2 = 7E5, 3 = 7E7 */
    volatile uint8_t *sio = lobo_io + 0x7E1 + 2 * minor;
    if (minor == 1)
        return TTY_READY_NOW;
    /* SIO is a bit messier */
    irq = di();
    *sio = 0;
    r = *sio;
    irqrestore(irq);
    if (r & 4)	/* THRE */
        return TTY_READY_NOW;
    /* TODO RTS/CTS */
    return TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    irqflags_t irq;
    uint8_t ch = c;
    if (minor == 1) {
       irq = di();
       vtoutput(&ch, 1);
       irqrestore(irq);
   } else if (minor == 2)
       lobo_io[0x7E4] = c;
    else
       lobo_io[0x7E6] = c;
}

static uint_fast8_t old_r2, old_r3;

void tty_interrupt(void)
{
    /* SIO.. check r & 2 case */
    uint_fast8_t r = lobo_io[0x7E5];
    if (r & 1)
        tty_inproc(2, lobo_io[0x7E4]);
    if ((r ^ old_r2) & 8) {
        old_r2 = r;
        if (r & 8)
            tty_carrier_raise(2);
        else
            tty_carrier_drop(2);
    }
    r = lobo_io[0x7E7];
    if (r & 1)
        tty_inproc(3, lobo_io[0x7E6]);
    if ((r ^ old_r3) & 8) {
        old_r3 = r;
        if (r & 8)
            tty_carrier_raise(3);
        else
            tty_carrier_drop(3);
    }
}

static const uint_fast8_t com8116[] = {
    0xE,
    0x0,		/* 50 */
    0x1,		/* 75 */
    0x2,		/* 110 */
    0x3,		/* 134.5 */
    0x4,		/* 150 */
    0x5,		/* 300 */
    0x6,		/* 600 */
    0x9,		/* 1200 */
    0xA,		/* 2400 */
    0xC,		/* 4800 */
    0xE,		/* 9600 */
    0xF			/* 19200 */
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    register struct tty *t = ttydata + minor;
    uint_fast8_t baud;
    uint_fast8_t ctrl;
    uint_fast8_t r;
    if (minor == 1)
        return;
    baud = t->termios.c_cflag & CBAUD;
    if (baud > B19200) {
        t->termios.c_cflag &= ~CBAUD;
        t->termios.c_cflag |= B19200;
        baud = B19200;
    }
    /* Our baud codes almost match up but not quite so use a table */
    /* 7D0 / 7D4 minor 2 and 3 */
    lobo_io[0x7C8 + 4 * minor] = com8116[baud];
    
    /* Set bits per character */
    sio_r[1] = 0x01 | ((t->termios.c_cflag & CSIZE) << 2);

    r = 0xC4;
	if (t->termios.c_cflag & CSTOPB)
		r |= 0x08;
	if (t->termios.c_cflag & PARENB)
		r |= 0x01;
	if (t->termios.c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->termios.c_cflag & CSIZE) << 1);
    sio2_out(minor);
}

int tty_carrier(uint_fast8_t minor)
{	
    irqflags_t irq;
    /* 2 = 7E5, 3 = 7E7 */
    volatile uint8_t *sio = lobo_io + 0x7E1 + 2 * minor;
    uint_fast8_t r;

    if (minor == 1)
        return 1;

    irq = di();
    *sio = 0;
    r = *sio;
    irqrestore(irq);

    if (r & 0x08)
        return 1;
    return 0;
}

void tty_sleeping(uint_fast8_t minor)
{
        used(minor);
}

void tty_data_consumed(uint_fast8_t minor)
{
    /* FIXME: flow control */
}

uint_fast8_t keymap[8];
static uint_fast8_t keyin[8];
static uint_fast8_t keybyte, keybit;
static uint_fast8_t newkey;
static int keysdown = 0;
static uint_fast8_t shiftmask[8] = {
    0, 0, 0, 0, 0, 0, 0, 7
};

static void keyproc(void)
{
	int i;
	uint_fast8_t key;

	for (i = 0; i < 8; i++) {
	        /* Set one of A0 to A7, and read the byte we get back.
	           Invert that to get a mask of pressed buttons */
		keyin[i] = *(uint_fast8_t *)(0x0800 | (1 << i));
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

uint_fast8_t keyboard[8][8] = {
	{'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g' },
	{'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o' },
	{'p', 'q', 'r', 's', 't', 'u', 'v', 'w' },
	{'x', 'y', 'z', '[', '\\', ']', '^', '_' },
	{'0', '1', '2', '3', '4', '5', '6', '7' },
	{'8', '9', ':', ';', ',', '-', '.', '/' },
	{ KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, KEY_DOWN, KEY_BS, KEY_DEL, ' '},
	{ 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_ESC, 0, 0 }
};

uint_fast8_t shiftkeyboard[8][8] = {
	{'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G' },
	{'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O' },
	{'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W' },
	{'X', 'Y', 'Z', '{', '|', '}', '^', '_' },
	{'0', '!', '"', '#', '$', '%', '&', '\'' },
	{'(', ')', '*', '+', '<', '=', '>', '?' },
	{ KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, ' '},
	{ 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_ESC, 0, 0 }
};

static uint_fast8_t capslock = 0;
static uint_fast8_t kbd_timer;

static void keydecode(void)
{
	uint_fast8_t c;
	uint_fast8_t m = 0;

	if (keybyte == 7 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[7] & 1) {	/* shift */
	        m = KEYPRESS_SHIFT;
		c = shiftkeyboard[keybyte][keybit];
        } else
		c = keyboard[keybyte][keybit];

        /* The keyboard lacks some rather important symbols so remap them
           with control */
	if (keymap[7] & 0x80) {	/* control */
	        m |= KEYPRESS_CTRL;
                if (keymap[7] & 1) {	/* shift */
                    if (c == '(')
                        c = '{';
                    if (c == ')')
                        c = '}';
                    if (c == '-')
                        c = '_';
                    if (c == '/')
                        c = '`';
                    if (c == '<')
                        c = '^';
                } else {
                    if (c == '8')
                        c = '[';
                    else if (c == ')')
                        c = '9';
                    else if (c == '-')
                        c = '|';
                    else if (c > 31 && c < 127)
			c &= 31;
                }
	}
	else if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c) {
	        switch(keyboard_grab) {
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

void kbd_interrupt(void)
{
	newkey = 0;
	if (!keysdown && lobo_io[0x08FF] == 0)
		return;
	keyproc();
	if (keysdown && keysdown < 3) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first;
		} else if (! --kbd_timer) {
			keydecode();
			kbd_timer = keyrepeat.continual;
		}
	}
}

/* TODO: move to asm etc */

static volatile unsigned char *cpos;
static unsigned char csave;
static unsigned crtbase;		/* Base written into CRTC at this point */

/* The display is split on a 1K boundary */
static volatile uint8_t *char_addr(unsigned int y1, unsigned char x1)
{
	unsigned off = VT_WIDTH * y1 + x1;
	off += crtbase;
	off &= 0x07FF;
	if (off < 0x0400)
		return 0x0C00 + off;
	else
		return off - 0x0400;	/* so 0x0000-0x03FF */
}

void cursor_off(void)
{
	if (cpos)
		*cpos = csave;
}

/* Only needed for hardware cursors */
void cursor_disable(void)
{
}

/* Make symbol 127 the inverse of the symbol under the cursor */
static void composite_cursor(void)
{
	register volatile uint8_t *cp = (uint8_t *)(127 * 8);
	register volatile uint8_t *sp = (uint8_t *)(csave * 8);
	irqflags_t irq;
	uint_fast8_t old;
	register unsigned ct = 0;

	irq = di();
	old = lobo_io[0x7DC];
	lobo_io[0x7DC] = (old & 0xF8) | 4;
	while(ct++ < 8)
		*cp++ = ~*sp++;
	lobo_io[0x7DC] = old;
	irqrestore(irq);
}

void cursor_on(int8_t y, int8_t x)
{
	cpos = char_addr(y, x);
	csave = *cpos;
	composite_cursor();
	*cpos = 127;		/* used as the cursor composited char */
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	*char_addr(y, x) = c;
}

void clear_lines(int8_t y, register int8_t ct)
{
	/* Skip being clever for now. The join rolls */
	while(ct--)
		clear_across(y, 0, VT_WIDTH);
}

void clear_across(int8_t y, register int8_t x, register int16_t l)
{
	while(l--)
		*char_addr(y, x++) = 0x20;
}

void vtattr_notify(void)
{
}

static void crt_set(void)
{
	irqflags_t irq;

	while(!(lobo_io[0x07E0] & 0x40));

	crtbase &= 0x07FF;
	irq = di();
	lobo_io[0x7E0] = 12;
	lobo_io[0x7E1] = crtbase >> 8;
	lobo_io[0x7E0] = 13;
	lobo_io[0x7E1] = crtbase;
	irqrestore(irq);
}

void scroll_up(void)
{
	crtbase += VT_WIDTH;
	/* Wipe the memory we will expose otherwise you sometimes
	   get an annoying flash bottom right */
	clear_across(23, 0, VT_WIDTH);
	crt_set();
}

void scroll_down(void)
{
	crtbase -= VT_WIDTH;
	crt_set();
}

/* This is a pain... half our font is in a different format. Dump that
   on the user and just declare 8x16 : TODO */
static const struct fontinfo fonti = {
	0, 128, 128, 191, FONT_INFO_8X16
};

static void lobo_set_char8(uint_fast8_t c, uint8_t *ptr)
{
	register uint8_t *ioa = lobo_io + c * 8;
	irqflags_t irq = di();
	uint_fast8_t old = lobo_io[0x7DC];
	lobo_io[0x7DC] = (old & 0xF8) | 4;
	memcpy(ioa, ptr, 8);
	lobo_io[0x7DC] = old;
	irqrestore(irq);
}

static void lobo_set_char16(uint_fast8_t c, uint8_t *ptr)
{
	register uint8_t *ioa = lobo_io + (c & 0x3F) * 16;
	irqflags_t irq = di();
	uint_fast8_t old = lobo_io[0x7DC];
	lobo_io[0x7DC] = (old & 0xF8) | 6;
	memcpy(ioa, ptr, 16);
	lobo_io[0x7DC] = old;
	irqrestore(irq);
}

int lobotty_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	register uint_fast8_t i;
	uint8_t map[16];

	if (minor != 1)
		return tty_ioctl(minor, arg, ptr);

	switch(arg) {
	case VTFONTINFO:
		return uput(&fonti, ptr, sizeof(struct fontinfo));
	case VTSETUDG:
		i = ugetc(ptr);
		ptr++;
		if (i < 128 || i > 191) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, map, 16) == -1) {
			udata.u_error = EFAULT;
			return -1;
		}
		lobo_set_char16(i, map);
		return 0;
	case VTSETFONT:
		i = 0;
		memset(map + 8, 0, 8);
		while(i < 128) {
			if (uget(ptr, map, 8) == -1) {
				udata.u_error = EFAULT;
				return -1;
			}
			lobo_set_char8(i++, map);
			ptr += 16;
		}
		return 0;
	}
	return vt_ioctl(minor, arg, ptr);
}
