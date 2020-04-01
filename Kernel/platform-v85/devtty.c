#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

static unsigned char tbuf1[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[1].termios;
	uint_fast8_t r = t->c_cflag & CSIZE;
	/* No CS5/CS6 CS7 must have parity enabled */
	if (r <= CS7) {
		t->c_cflag &= ~CSIZE;
		t->c_cflag |= CS7|PARENB;
	}
	/* No CS8 parity and 2 stop bits */
	if (r == CS8 && (t->c_cflag & PARENB))
		t->c_cflag &= ~CSTOPB;
	/* There is no obvious logic to this */
	switch(t->c_cflag & (CSIZE|PARENB|PARODD|CSTOPB)) {
	case CS7|PARENB:
		r = 0xEB;
		break;
	case CS7|PARENB|PARODD:
		r = 0xEF;
		break;
	case CS7|PARENB|CSTOPB:
		r = 0xE3;
	case CS7|PARENB|PARODD|CSTOPB:
		r = 0xE7;
	case CS8|CSTOPB:
		r = 0xF3;
		break;
	case CS8:
		r = 0xF7;
		break;
	case CS8|PARENB:
		r = 0xFB;
		break;
	case CS8|PARENB|PARODD:
		r = 0xFF;
		break;
	}
	acia_setup(r);
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    minor;
    return 1;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    switch(minor){
        case 1:
            ttyout(c);
            break;
    }
}

void tty_sleeping(uint_fast8_t minor)
{
    minor;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    uint_fast8_t r;
    if (minor == 1)
        r = ttyready();
    if (r)
        return TTY_READY_NOW;
    return TTY_READY_SOON;
}

void tty_poll(void)
{
    uint16_t r;
    while((r = acia_poll()) != 0xFFFF)
        tty_inproc(1, r);
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}
