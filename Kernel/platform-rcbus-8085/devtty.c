#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <graphics.h>
#include <rcbus.h>

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
};

static uint_fast8_t acia_minor;
static uint_fast8_t uart_minor;
static uint_fast8_t num_uart;

/* Mostly not used yet but needed by VDP code for multi console */
uint8_t inputtty = 1;
uint8_t outputtty = 1;
uint8_t vtattr_cap;
uint8_t vidmode;

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

/*
 *	16x50 conversion betwen a Bxxxx speed rate (see tty.h) and the values
 *	to stuff into the chip.
 */
static const uint16_t clocks[] = {
	12,		/* Not a real rate */
	2304,
	1536,
	1047,
	857,
	768,
	384,
	192,
	96,
	48,
	24,
	12,
	6,
	3,
	2,
	1
};

static void calc_16x50_setup(void)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[uart_minor].termios;

	d = 0x80;	/* DLAB (so we can write the speed) */
	d |= (t->c_cflag & CSIZE) >> 4;
	if(t->c_cflag & CSTOPB)
		d |= 0x04;
	if (t->c_cflag & PARENB)
		d |= 0x08;
	if (!(t->c_cflag & PARODD))
		d |= 0x10;
	out(0xC3, d);	/* LCR */
	w = clocks[t->c_cflag & CBAUD];
	out(0XC0, w);		/* Set the DL */
	out(0xC1, w >> 8);
	if (w >> 8)	/* Low speeds interrupt every byte for latency */
		out(0xC2, 0x00);
	else		/* High speeds set our interrupt quite early
			   as our latency is poor, turn on 64 byte if
			   we have a 16C750 */
		out(0xC2, 0x51);
	out(0xC3, d & 0x7F);
	/* FIXME: CTS/RTS support */
	out(0xC4, 0x03); /* DTR RTS */
	out(0xC1, 0x0D); /* We don't use tx ints */
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	if (minor == acia_minor) {
	  calc_acia_setup();
	} else {
	  calc_16x50_setup();
	}
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
  uint8_t ch = c;
  /* Shadow the first console on the TMS 9918A */
  if (minor == 1 && tms9918a_present)
	vtoutput(&ch, 1);
  if (minor == acia_minor)
    ttyout_acia(c);
  else
    ttyout_uart(c);
}

void tty_sleeping(uint_fast8_t minor)
{
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
   ports so far we need delays on the RCBUS */
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

int rctty_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
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

void do_beep(void)
{
}

void cursor_disable(void)
{
}
