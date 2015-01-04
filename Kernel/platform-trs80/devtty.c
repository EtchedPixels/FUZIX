#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <stdarg.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

__sfr __at 0xE8 tr1865_ctrl;
__sfr __at 0xE9 tr1865_baud;
__sfr __at 0xEA tr1865_status;
__sfr __at 0xEB tr1865_rxtx;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 }
};

/* Write to system console */
void kputchar(char c)
{
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

bool tty_writeready(uint8_t minor)
{
    uint8_t reg;
    if (minor == 1)
        return 1;
    reg = tr1865_status;
    if (reg & 0x40)
        return 1;
    return 0;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    if (minor == 1)
        vtoutput(&c, 1);
    if (minor == 2)
        tr1865_rxtx = c;
}

void tty_interrupt(void)
{
    uint8_t reg = tr1865_status;
    if (reg & 0x80) {
        reg = tr1865_rxtx;
        tty_inproc(2, reg);
    }
}

/* Called to set baud rate etc */
static const uint8_t trsbaud[] = {
    0,0,1,2, 3,4,5,6, 7,10,14, 15
};

static const uint8_t trssize[4] = {
    0x00, 0x40, 0x20, 0x60
};

void tty_setup(uint8_t minor)
{
    uint8_t baud;
    uint8_t ctrl;
    if (minor == 1)
        return;
    baud = ttydata[2].termios.c_cflag & CBAUD;
    if (baud > B19200) {
        ttydata[2].termios.c_cflag &= ~CBAUD;
        ttydata[2].termios.c_cflag |= B19200;
        baud = B19200;
    }
    tr1865_baud = baud | (baud << 4);

    ctrl = 3;
    if (ttydata[2].termios.c_cflag & PARENB) {
        if (ttydata[2].termios.c_cflag & PARODD)
            ctrl |= 0x80;
    } else
        ctrl |= 0x8;		/* No parity */
    ctrl |= trssize[(ttydata[2].termios.c_cflag & CSIZE) >> 4];
    tr1865_ctrl = ctrl;
}

int trstty_close(uint8_t minor)
{
    if (minor == 2)
        tr1865_ctrl = 0;	/* Drop carrier */
    return tty_close(minor);
}

int tty_carrier(uint8_t minor)
{
    if (minor == 1)
        return 1;
    if (tr1865_ctrl & 0x80)
        return 1;
    return 0;
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
		keyin[i] = *(uint8_t *)(0xF400 | (1 << i));
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown--;
				}
				if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown++;
					keybyte = i;
					keybit = n;
					newkey = 1;
				}
				m += m;

			}
		}
		keymap[i] = keyin[i];
	}
}

uint8_t keyboard[8][8] = {
	{'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g' },
	{'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o' },
	{'p', 'q', 'r', 's', 't', 'u', 'v', 'w' },
	{'x', 'y', 'z', '[', '\\', ']', '^', '_' },
	{'0', '1', '2', '3', '4', '5', '6', '7' },
	{'8', '9', ':', ';', ',', '-', '.', '/' },
	{13, 12, 3, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, ' '},
	{ 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, 0 }
};

uint8_t shiftkeyboard[8][8] = {
	{'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G' },
	{'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O' },
	{'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W' },
	{'X', 'Y', 'Z', '{', '|', '}', '^', '_' },
	{'0', '!', '"', '#', '$', '%', '&', '\'' },
	{'(', ')', '*', '+', '<', '=', '>', '?' },
	{13, 12, 3, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, ' '},
	{ 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, 0 }
};

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 7 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[7] & 3)	/* shift */
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];

        /* The keyboard lacks some rather important symbols so remap them
           with control */
	if (keymap[7] & 4) {	/* control */
		if (c > 31 && c < 96)
			c &= 31;
                if (keymap[7] & 3) {
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
                    if (c == '(')
                        c = '[';
                    if (c == ')')
                        c = ']';
                    if (c == '-')
                        c = '|';
                }
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c)
		vt_inproc(1, c);
}

void kbd_interrupt(void)
{
	newkey = 0;
	keyproc();
	if (keysdown < 3 && newkey)
		keydecode();
}

