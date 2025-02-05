#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <input.h>
#include <devinput.h>
#include <stdarg.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];

uint8_t curtty;		/* output side */
uint8_t inputtty;	/* input side */
static struct vt_switch ttysave[2];
static uint8_t vtbackbuf[VT_WIDTH * VT_HEIGHT];
struct vt_repeat keyrepeat;

uint8_t *vtbase[2] = { 0xF800, vtbackbuf };

__sfr __at 0xE8 tr1865_ctrl;
__sfr __at 0xE9 tr1865_baud;
__sfr __at 0xEA tr1865_status;
__sfr __at 0xEB tr1865_rxtx;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 }
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	/* FIXME CTS/RTS, CSTOPB ? */
	CSIZE|CBAUD|PARENB|PARODD|_CSYS
};

/* Write to system console */
void kputchar(char c)
{
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
    uint8_t reg;
    if (minor != 3)
        return TTY_READY_NOW;
    reg = tr1865_status;
    return (reg & 0x40) ? TTY_READY_NOW : TTY_READY_SOON;
}

void vtbuf_init(void)
{
    memset(vtbackbuf, ' ', VT_WIDTH * VT_HEIGHT);
}

void vtexchange(void)
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
        vt_cursor_off();

        /* Swap the buffers over */
        __asm
                ld hl, #0xf800
                ld de, #_vtbackbuf
                ld bc, #VT_WIDTH*VT_HEIGHT
        exchit:
                push bc
                ld a, (de)	; Could be optimised but its only 2K
                ld c, (hl)	; Probably worth doing eventuallly
                ex de, hl
                ld (hl), c
                ld (de), a
                inc hl
                inc de
                pop bc
                dec bc
                ld a, b
                or c
                jr nz, exchit
                ret
        __endasm;
        /* Cursor back */
        if (!ttysave[inputtty].cursorhide)
            cursor_on(ttysave[inputtty].cursory, ttysave[inputtty].cursorx);
}

void tty_putc(uint8_t minor, unsigned char c)
{
    irqflags_t irq;
    if (minor == 3)
        tr1865_rxtx = c;
    else {
        irq = di();
        if (curtty != minor -1) {
            /* Kill the cursor as we are changing the memory buffers. If
               we don't do this the next cursor_off will hit the wrong
               buffer */
            cursor_off();
            vt_save(&ttysave[curtty]);
            curtty = minor - 1;
            vt_load(&ttysave[curtty]);
            /* Fix up the cursor */
            if (!ttysave[curtty].cursorhide)
                cursor_on(ttysave[curtty].cursory, ttysave[curtty].cursorx);
       }
       vtoutput(&c, 1);
       irqrestore(irq);
   }
}

void tty_interrupt(void)
{
    /* TODO: carrier change handling */
    uint8_t reg = tr1865_status;
    if (reg & 0x80) {
        reg = tr1865_rxtx;
        tty_inproc(3, reg);
    }
}

/* Called to set baud rate etc */
static const uint8_t trsbaud[] = {
    0,0,1,2, 3,4,5,6, 7,10,14, 15
};

static const uint8_t trssize[4] = {
    0x00, 0x40, 0x20, 0x60
};

void tty_setup(uint8_t minor, uint8_t flags)
{
    uint8_t baud;
    uint8_t ctrl;
    if (minor != 3)
        return;
    baud = ttydata[3].termios.c_cflag & CBAUD;
    if (baud > B19200) {
        ttydata[3].termios.c_cflag &= ~CBAUD;
        ttydata[3].termios.c_cflag |= B19200;
        baud = B19200;
    }
    baud = trsbaud[baud];
    tr1865_baud = baud | (baud << 4);

    ctrl = 3;
    if (ttydata[3].termios.c_cflag & PARENB) {
        if (ttydata[3].termios.c_cflag & PARODD)
            ctrl |= 0x80;
    } else
        ctrl |= 0x8;		/* No parity */
    ctrl |= trssize[(ttydata[3].termios.c_cflag & CSIZE) >> 4];
    tr1865_ctrl = ctrl;
}

int trstty_close(uint8_t minor)
{
    if (minor == 3 && ttydata[3].users == 0)
        tr1865_ctrl = 0;	/* Drop carrier */
    return tty_close(minor);
}

int tty_carrier(uint8_t minor)
{
    if (minor != 3)
        return 1;
    if (tr1865_ctrl & 0x80)
        return 1;
    return 0;
}

void tty_sleeping(uint8_t minor)
{
        used(minor);
}

void tty_data_consumed(uint8_t minor)
{
    /* FIXME: flow control as implemented now for Model I and III */
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

uint8_t keyboard[8][8] = {
#ifdef CONFIG_AZERTY
	{'>', 'q', 'b', 'c', 'd', 'e', 'f', 'g' },
	{'h', 'i', 'j', 'k', 'l', ',', 'n', 'o' },
	{'p', 'a', 'r', 's', 't', 'u', 'v', 'z' },
	{'x', 'y', 'w', '^', '@', 'm', '\\','}' },
	{'}', '&', '[', '"', '\'','(', '|', ']' },
	{'!', '{', ')', '-', '$', ';', ':', '=' },
	{ KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, KEY_DOWN, KEY_BS, KEY_DEL, ' '},
	{ 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, 0 }
#else
	{'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g' },
	{'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o' },
	{'p', 'q', 'r', 's', 't', 'u', 'v', 'w' },
	{'x', 'y', 'z', '[', '\\', ']', '^', '_' },
	{'0', '1', '2', '3', '4', '5', '6', '7' },
	{'8', '9', ':', ';', ',', '-', '.', '/' },
	{ KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, KEY_DOWN, KEY_BS, KEY_DEL, ' '},
	{ 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, 0 }
#endif
};

uint8_t shiftkeyboard[8][8] = {
#ifdef CONFIG_AZERTY
	{'<', 'Q', 'B', 'C', 'D', 'E', 'F', 'G' },
	{'H', 'I', 'J', 'K', 'L', '?', 'N', 'O' },
	{'P', 'A', 'R', 'S', 'T', 'U', 'V', 'Z' },
	{'X', 'Y', 'W', '`', '*', 'M', '%', '{' },
	{'0', '1', '2', '3', '4', '5', '6', '7' },
	{'8', '9', '~', '_', '#', '.', '/', '+' },
	{ KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, ' '},
	{ 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, 0 }
#else
	{'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G' },
	{'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O' },
	{'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W' },
	{'X', 'Y', 'Z', '{', '|', '}', '^', '_' },
	{'0', '!', '"', '#', '$', '%', '&', '\'' },
	{'(', ')', '*', '+', '<', '=', '>', '?' },
	{ KEY_ENTER, KEY_CLEAR, KEY_STOP, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, ' '},
	{ 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, 0 }
#endif
};

static uint8_t capslock = 0;
static uint8_t kbd_timer;

static void keydecode(void)
{
	uint8_t c;
	uint8_t m = 0;

	if (keybyte == 7 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[7] & 3) {	/* shift */
	        m = KEYPRESS_SHIFT;
		c = shiftkeyboard[keybyte][keybit];
		/* VT switcher */
		if (c == KEY_F1 || c == KEY_F2) {
                        if (inputtty != c - KEY_F1) {
                                inputtty = c - KEY_F1;
                                vtexchange();	/* Exchange the video and backing buffer */
                        }
                        return;
                }
        } else
		c = keyboard[keybyte][keybit];

        /* The keyboard lacks some rather important symbols so remap them
           with control */
	if (keymap[7] & 4) {	/* control */
	        m |= KEYPRESS_CTRL;
#ifdef CONFIG_AZERTY
                if (!(keymap[7] & 3)) {	/* no shift */
                    if (c == '&')
                        c = '|';
                    else if (c == '^')
                        c = '[';
                    else if (c == '@')
                        c = ']';
                    else if (c == '=')
                        c = '~';
                    else if (c == '$')
                        c = '`';
                    else if (c == '>')
                        c = '\\';
                    else if (c > 31 && c < 127)
			c &= 31;
		}
#else
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
                    if (c == '8')
                        c = '[';
                    else if (c == ')')
                        c = '9';
                    else if (c == '-')
                        c = '|';
                    else if (c > 31 && c < 127)
			c &= 31;
                }
#endif
	}
	else if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c) {
	        switch(keyboard_grab) {
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

void kbd_interrupt(void)
{
	newkey = 0;
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
