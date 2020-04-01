#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <graphics.h>
#include <rc2014.h>

static unsigned char tbuf1[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

static uint_fast8_t acia_minor;
static uint_fast8_t uart_minor;
static uint_fast8_t num_uart;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/*CSIZE|CSTOPB|PARENB|PARODD|*/_CSYS,
	_CSYS
};

/* ACIA is always port 1 if present */
static void calc_acia_setup(void)
{
	struct termios *t = &ttydata[1].termios;
	uint8_t r = t->c_cflag & CSIZE;

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
		r = 0x8A;
		break;
	case CS7|PARENB|PARODD:
		r = 0x8E;
		break;
	case CS7|PARENB|CSTOPB:
		r = 0x82;
	case CS7|PARENB|PARODD|CSTOPB:
		r = 0x86;
	case CS8|CSTOPB:
		r = 0x92;
		break;
	default:
	case CS8:
		r = 0x96;
		break;
	case CS8|PARENB:
		r = 0x9A;
		break;
	case CS8|PARENB|PARODD:
		r = 0x9E;
		break;
	}
	acia_setup(r);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	if (minor == acia_minor) {
	  calc_acia_setup();
	} else {
  	  /* TODO */
	}
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
	minor;
	return 1;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
  if (minor == acia_minor)
    ttyout_acia(c);
  else
    ttyout_uart(c);
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
    if (minor == acia_minor)
        r = acia_ready();
    else
        r = uart_ready();
    if (r)
        return TTY_READY_NOW;
    return TTY_READY_SOON;
}

void tty_poll(void)
{
    uint16_t r;
    if (uart_minor)
      while((r = uart_poll()) != 0xFFFF)
          tty_inproc(uart_minor, r);
    if (acia_minor)
      while((r = acia_poll()) != 0xFFFF)
          tty_inproc(acia_minor, r);
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}

/*
 *	Graphics layer interface (for TMS9918A)
 */

static const struct videomap tms_map = {
	0,
	0x98,
	0, 0,
	0, 0,
	1,
	MAP_PIO
};

/* FIXME: we need a way of reporting CPU speed/TMS delay info as unlike the
   ports so far we need delays on the RC2014 */
static const struct display tms_mode = {
    1,
    256, 192,
    256, 192,
    255, 255,
    FMT_VDP,
    HW_VDP_9918A,
    GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
    16,
    0
};

int rctty_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  if (minor < 3 && tms9918a_present) {
 	switch(arg) {
 	case GFXIOC_GETINFO:
 		return uput(&tms_mode, ptr, sizeof(struct display));
	case GFXIOC_MAP:
	        return uput(&tms_map, ptr, sizeof(struct display));
	case GFXIOC_UNMAP:
		return 0;
	}
  }
  return tty_ioctl(minor, arg, ptr);
}

int rctty_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor <= num_uart)
		return tty_open(minor, flag);
	udata.u_error = ENXIO;
	return -1;
}

void rctty_init(void)
{
  if (acia_present) {
    acia_minor = ++num_uart;
    termios_mask[1] = _CSYS|CSIZE|CSTOPB|PARENB|PARODD;
  }
  if (uart_present) {
    uart_minor = ++num_uart;
    /* TODO: set up and termios mask */
  }
}
