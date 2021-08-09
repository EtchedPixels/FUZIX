#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <graphics.h>
#include <devtty.h>
#include <vt.h>
#include <vdp1.h>
#include <z180.h>
#include "n8.h"

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];
static uint8_t tbuf3[TTYSIZ];
static uint8_t tbuf4[TTYSIZ];
static uint8_t tbuf5[TTYSIZ];
static uint8_t tbuf6[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf4,   tbuf4,   tbuf4,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf5,   tbuf5,   tbuf5,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf6,   tbuf6,   tbuf6,   TTYSIZ,   0,   TTYSIZ/2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS | CBAUD | PARENB | PARODD | CSIZE | CSTOPB | CRTSCTS,
	_CSYS | CBAUD | PARENB | PARODD | CSIZE | CSTOPB | CRTSCTS
};

uint8_t vidmode;	/* For live screen */
static uint8_t mode[5];	/* Per console */
static uint8_t vswitch;	/* Track vt switch locking due top graphics maps */
uint8_t vt_twidth;
uint8_t vt_tright;
uint8_t outputtty = 1, inputtty = 1;
#define VT_CON	4
static struct vt_switch ttysave[VT_CON];
uint8_t vtattr_cap;

/* bit 5: turn on divide by 30 v 10
   bit 3: turn on scale by 64 not 16
   bit 2-0: 2^n for scaling (not 111) */
static const uint8_t baudtable[] = {
	/* Dividers for our clock. Table is smaller than the maths by far */
	0,
	0,			/* 50 */
	0,			/* 75 */
	0,			/* 110 */
	0,			/* 134.5 */
	0x2E,			/* 150 */
	0x2D,			/* 300 */
	0x2C,			/* 600 */
	0x2B,			/* 1200 */
	0x2A,			/* 2400 */
	0x29,			/* 4800 */
	0x28,			/* 9600 */
	/* Now switch to 16x clock */
	0x21,			/* 19200 */
	0x20,			/* 38400 */
	/* And 10x scaler */
	0x01,			/* 57600 */
	0x00,			/* 115200 */
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t cntla = 0x60;
	uint8_t cntlb = 0;
	uint16_t cflag = t->c_cflag;
	uint8_t baud;
	uint8_t ecr = 0;

	used(flags);

	/* TMS9918A consoles */
	if (minor <= 4)
		return;
	/* Which channel ? */
	minor -= 4;

	/* Calculate the control bits */
	if (cflag & PARENB) {
		cntla |= 2;
		if (cflag & PARODD)
			cntlb |= 4;
	}
	if ((cflag & CSIZE) == CS8)
		cntla |= 4;
	else {
		cflag &= ~CSIZE;
		cflag |= CS7;
	}
	if (cflag & CSTOPB)
		cntla |= 1;

	/* Handle the baud table. Right now this is hardcoded for our clock */

	baud = cflag & CBAUD;
	/* We can't get below 150 easily. We might be able to do this with the
	   BRG on one channel - need to check FIXME */
	if (baud && baud < B150) {
		baud = B150;
		cflag &= ~CBAUD;
		cflag |= B150;
	}
	cntlb |= baudtable[baud];

	if (minor == 1) {
		if (cflag & CRTSCTS)
			ecr = 0x20;
		/* FIXME: need to do software RTS side */
	} else {
		cflag &= ~CRTSCTS;
	}

	t->c_cflag = cflag;

	/* ASCI serial set up */
	if (minor == 1) {
		ASCI_CNTLA0 = cntla;
		ASCI_CNTLB0 = cntlb;
		ASCI_ASEXT0 &= ~0x20;
		ASCI_ASEXT1 |= ecr;
	} else if (minor == 2) {
		ASCI_CNTLA1 = cntla;
		ASCI_CNTLB1 = cntlb;
	}
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    minor;
    return 1;
}

void tty_pollirq_asci0(void)
{
    while(ASCI_STAT0 & 0x80)
        tty_inproc(1, ASCI_RDR0);
}

void tty_pollirq_asci1(void)
{
    while(ASCI_STAT1 & 0x80)
        tty_inproc(2, ASCI_RDR1);
}

static void tms_setoutput(uint_fast8_t minor)
{
	vt_save(&ttysave[outputtty - 1]);
	outputtty = minor;
	vt_load(&ttysave[outputtty - 1]);
}

static void tms_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

	if (outputtty != minor) {
		ASCI_TDR0 = '@';
		tms_setoutput(minor);
	}
	irqrestore(irq);
	if (!vswitch)
		vtoutput(&c, 1);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    switch(minor){
    	case 1:
	// TOOD DEBUG HACK
            while(!(ASCI_STAT0 & 2));
            ASCI_TDR0 = c;
    	case 2:
    	case 3:
    	case 4:
	    tms_putc(minor, c);
	    break;
        case 5:
            while(!(ASCI_STAT0 & 2));
            ASCI_TDR0 = c;
            break;
        case 6:
            while(!(ASCI_STAT1 & 2));
            ASCI_TDR1 = c;
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
    minor;
    /* FIXME: */
    return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}

/*
 *	Graphics layer interface (for TMS9918A and friends)
 */

static struct videomap tms_map = {
	0,
	0x98,
	0, 0,
	0, 0,
	1,
	MAP_PIO
};

/* FIXME: we need a way of reporting CPU speed/TMS delay info as unlike the
   ports so far we need delays on the RC2014 */

/*
 *	We always report a TMS9918A. Now it is possible that someone
 *	is using a 9938 or 9958 so we might want to add some VDP autodetect
 *	code and report accordingly but that's a minor TODO
 */
static struct display tms_mode[2] = {
	{
		0,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
		16,
		0,
		40, 24
	},
	{
		1,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
		16,
		0,
		32, 24
	}
};

void vdp_reload(void)
{
	irqflags_t irq = di();

	if (mode[inputtty] == 0) {
		vdp_setup40();
		vt_twidth = 40;
		vt_tright = 39;
	} else {
		vdp_setup32();
		vt_twidth = 32;
		vt_tright = 31;
	}
	vidmode = mode[inputtty];
	vdp_restore_font();
	vt_cursor_off();
	vt_cursor_on();
	kputs("vdpreload");
	irqrestore(irq);
}

/* We can have up to 4 vt consoles or it may be shadowing serial input */
int n8tty_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  uint8_t map[8];
  if (minor <= 4) {
  	switch(arg) {
  	case GFXIOC_GETINFO:
                return uput(&tms_mode[mode[minor]], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		if (vswitch)
			return -EBUSY;
		vswitch = minor;
		return uput(&tms_map, ptr, sizeof(struct display));
	case GFXIOC_UNMAP:
		if (vswitch == minor) {
			vdp_reload();
			vswitch = 0;
		}
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE: {
		uint8_t m = ugetc(ptr);
		if (m > 1) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(&tms_mode[m], ptr, sizeof(struct display));
		mode[minor] = m;
		if (minor == inputtty)
			vdp_reload();
		return 0;
		}
	case VDPIOC_SETUP:
		/* Must be locked to issue */
		if (vswitch != minor) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, map, 8) == -1)
			return -1;
		map[1] |= 0x80;
		vdp_setup(map);
		return 0;
	case VDPIOC_READ:
	case VDPIOC_WRITE:
	{
		struct vdp_rw rw;
		uint16_t size;
		uint8_t *addr = (uint8_t *)rw.data;
		
		if (vswitch != minor) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, &rw, sizeof(struct vdp_rw)) != sizeof(struct vdp_rw)) {
			udata.u_error = EFAULT;
			return -1;
		}
		size = rw.lines * rw.cols;
		if (valaddr(addr, size) != size) {
			udata.u_error = EFAULT;
			return -1;
		}
		if (arg == VDPIOC_READ)
			udata.u_error = vdp_rop(&rw);
		else
			udata.u_error = vdp_wop(&rw);
		if (udata.u_error)
			return -1;
		return 0;
	}
	}
	return vt_ioctl(minor, arg, ptr);
  }
  /* Not a VT port so don't go via generic VT */
  return tty_ioctl(minor, arg, ptr);
}

int n8tty_close(uint_fast8_t minor)
{
	if (vswitch == minor) {
		vdp_reload();
		vswitch = 0;
	}
	return tty_close(minor);
}
