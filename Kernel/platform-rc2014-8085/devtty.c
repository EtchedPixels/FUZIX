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

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/*CSIZE|CSTOPB|PARENB|PARODD|*/_CSYS,
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	/* TODO */
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
        r = uart_ready();
    if (r)
        return TTY_READY_NOW;
    return TTY_READY_SOON;
}

void tty_poll(void)
{
    uint16_t r;
    while((r = uart_poll()) != 0xFFFF)
        tty_inproc(1, r);
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
  if (minor == 1 && tms9918a_present) {
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
