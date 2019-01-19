#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

/* Console is only port we provide, port 2 there but used for floppies.
   Eventually wants supporting but needs some tricky handling of flow
   control as a single UART is switched multiple ways including for
   the fd */

static char tbuf1[TTYSIZ];

uint8_t vtattr_cap;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* Write to system console */
void kputchar(char c)
{
    /* handle CRLF */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

/* It's the console display, always ready */
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

void tty_setup(uint8_t minor)
{
    minor;
}

void tty_sleeping(uint8_t minor)
{
    minor;
}

int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

/*
 *	Keyboard interface. This is not a make break interface except for
 *	the shift keys. We don't support item keyboards just the standard
 *	ones.
 */

static uint8_t shifts = 0;
#define SHIFT_CTRL	(1 << 2)
#define SHIFT_LSHIFT	(1 << 3)
#define SHIFT_CAPSLOCK	(1 << 4)
#define SHIFT_GRPH	(1 << 5)
#define SHIFT_RSHIFT	(1 << 6)
#define SHIFT_NUM	(1 << 7)

/* Map table. Interleave original and shift as the low bits are never 8-15 */
static uint8_t keymap[] = {
    KEY_ESC, KEY_PAUSE, KEY_HELP, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
    KEY_ESC, KEY_PAUSE, KEY_HELP, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_STOP, '1', '2', '3', '4', '5', '6', '7',
    KEY_STOP, '!', '"', KEY_POUND, '$', '%', '&', '\'',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', '+',
    'b', 'n', 'm', ',', '.', '/', '[', ']',
    'B', 'N', 'M', '<', '>', '?', '{', '}',
    '8', '9', '0', '-', '^', KEY_UP, KEY_BS, KEY_TAB,
    '(', ')', '_', '=', '~', KEY_UP, KEY_HOME, KEY_TAB,
    'o', 'p', '@', KEY_LEFT, KEY_DOWN, KEY_RIGHT, 'a', 's',
    'O', 'P', '`', KEY_LEFT, KEY_DOWN, KEY_RIGHT, 'A', 'S',
    ':', KEY_ENTER, '\\', ' ', 'z', 'x', 'c', 'v',
    '*', KEY_ENTER, '|', ' ', 'Z', 'X', 'C', 'V',
    KEY_INSERT, KEY_DEL, 0, 0, 0, 0, 0, 0,
    KEY_PRINT, KEY_CLEAR
};

static uint8_t key_decoder(uint8_t c)
{
    if (shifts & (SHIFT_LSHIFT|SHIFT_RSHIFT))
        c |= 8;
    c = keymap[c];
    if (c >= 'a' && c <= 'z') {
        if (shifts & SHIFT_CTRL)
            c -= 64;
        else if (shifts & SHIFT_CAPSLOCK)
            c -= 32;
    } else if (c >= 'A' && c <= 'Z' && (shifts & SHIFT_CTRL))
        c -= 32;

    /* We don't use graph or num currently */
    return c;
}


void key_pressed(uint16_t code)
{
    uint8_t c = code;
    uint8_t clow = c & 7;

    if (c >= 0xB0)
        shifts |= clow;
    else if (c >= 0xA0)
        shifts &= ~clow;
    else if (c <= 0x81) {
        /* Typed symbol */
        c = key_decoder(c);	/* Consider shifts */
        if (c)
            tty_inproc(1, c);
    }
        
}