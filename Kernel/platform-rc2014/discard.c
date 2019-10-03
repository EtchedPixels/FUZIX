#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <ppide.h>
#include <rc2014.h>
#include <vt.h>
#include <netdev.h>
#include <zxkey.h>
#include "vfd-term.h"
#include "z180_uart.h"

/* Everything in here is discarded after init starts */

static const uint8_t tmstext[] = {
	0x00,
	0xD0,
	0x00,		/* Text at 0x0000 (space for 4 screens) */
	0x00,
	0x02,		/* Patterns at 0x1000 */
	0x00,
	0x00,
	0xF1		/* white on black */
};

static const uint8_t tmsreset[] = {
	0x00,
	0x80,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00
};

static void nap(void)
{
}

static void tmsconfig(uint8_t *r)
{
	uint8_t c = 0x80;
	while(c < 0x88) {
		tms9918a_ctrl = *r++;
		tms9918a_ctrl = c++;
		nap();
	}
}

extern uint8_t fontdata_6x8[];

static uint8_t probe_tms9918a(void)
{
	uint16_t ct = 0;
	uint8_t v;
	uint8_t *fp;

	/* Try turning it on and looking for a vblank */
	tmsconfig(tmsreset);
	tmsconfig(tmstext);
	/* Should see the top bit go high */
	do {
		v = tms9918a_ctrl & 0x80;
	} while(ct-- && !(v & 0x80));

	if (ct == 0)
		return 0;
	nap();

	/* Reading the F bit should have cleared it */
	v = tms9918a_ctrl;
	if (v & 0x80)
		return 0;

	/* We have a TMS9918A, load up the fonts */
	ct = 0;


	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x40 | 0x00;	/* Console 0 */
	while(ct++ < 4096) {
		tms9918a_data = ' ';
		nap();
	}

	fp = fontdata_6x8;
	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x40 | 0x11;	/* Base of character 32 */
	ct = 0;
	while(ct++ < 768) {
		tms9918a_data = *fp++ << 2;
		nap();
	}

	fp = fontdata_6x8;
	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x40 | 0x15;	/* Base of character 160 */
	ct = 0;
	/* Load inverse video font data */
	while(ct++ < 768) {
		tms9918a_data = ~(*fp++ << 2);
		nap();
	}

	/* Initialize the VT layer */
	vtinit();
	return 1;
}

static uint8_t probe_16x50(uint8_t p)
{
	uint8_t r;
	uint8_t lcr = in(p + 3);
	out(p + 3, lcr | 0x80);
	out(p + 1, 0xAA);
	if (in(p + 1) != 0xAA) {
		out(p + 3, lcr);
		return 0;
	}
	out (p + 3, lcr);
	if (in(p + 1) == 0xAA)
		return 0;

	out (p + 2, 0xE7);
	r = in(p + 2);
	if (r & 0x40) {
		/* Decode types with FIFO */
		if (r & 0x80) {
			if (r & 0x20)
				return 7;
			return 5;	/* 16550A */
		}
		/* Should never find this real ones were discontinued
		   very early due to a hardware bug */
		return 0;
	} else {
		/* Decode types without FIFO */
		out(p + 7, 0x2A);
		if (in (p + 7) == 0x2A)
			return 4;
		return 8;
	}
}

/* Look for a QUART at 0xBB */

#define QUARTREG(x)	(((x) << 11) | 0xBB)

#define MRA	0x00
#define IVR1	0x0C
#define IVR2	0x1C

static uint8_t probe_quart(void)
{
	uint8_t c = in16(QUARTREG(IVR1));

	/* Make sure we they don't appear to affect one another */
	out16(QUARTREG(IVR1), c + 1);
	if(in16(QUARTREG(IVR1)) != c + 1)
		return 0;
	if(in16(QUARTREG(MRA)) == c + 1)
		return 0;
	/* Ok now check IVR2 also works */
	out16(QUARTREG(IVR2), c + 2);
	if(in16(QUARTREG(IVR1)) != c + 1)
		return 0;
	if(in16(QUARTREG(IVR2)) != c + 2)
		return 0;
	return 1;
}

void init_hardware_c(void)
{
#ifdef CONFIG_VFD_TERM
	vfd_term_init();
#endif
	ramsize = 512;
	procmem = 512 - 80;

	tms9918a_present = probe_tms9918a();
	if (tms9918a_present)
		shadowcon = 1;

	/* FIXME: When ROMWBW handles 16550A or second SIO, or Z180 as
	   console we will need to address this better */
	if (z180_present) {
		z180_setup(!ctc_present);
		register_uart(Z180_IO_BASE, &z180_uart0);
		register_uart(Z180_IO_BASE + 1, &z180_uart1);
		rtc_port = 0x0C;
		rtc_shadow = 0x0C;
	}

	/* Set the right console for kernel messages */
	/* ROMWBW favours the SIO then the ACIA */
	if (sio_present) {
		register_uart(0x80, &sio_uart);
		register_uart(0x82, &sio_uartb);
	}
	if (acia_present)
		register_uart(0xA0, &acia_uart);
	if (quart_present) {
		register_uart(0x00BB, &quart_uart);
		register_uart(0x40BB, &quart_uart);
		register_uart(0x80BB, &quart_uart);
		register_uart(0xC0BB, &quart_uart);
	}
}

void pagemap_init(void)
{
	uint8_t i, m;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 32-63 (page size is 16 KiB)
	 * Pages 32-34 are used by the kernel
	 * Page 35 is the common area for init
	 * Page 36 is the disk cache
	 * Pages 37 amd 38 are the second kernel bank
	 */
	for (i = 32 + 7; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);

	/* Could be at 0xC0 or 0x0C */
	ds1302_init();
	if (!ds1302_present) {
		rtc_port = 0x0C;
		ds1302_init();
	}

	/* Further ports we register at this point */
	if (sio1_present) {
		register_uart(0x84, &sio_uart);
		register_uart(0x86, &sio_uartb);
	}
	if (ctc_present)
		kputs("Z80 CTC detected at 0x88.\n");

	if (tms9918a_present)
		kputs("TMS9918A VDP detected at 0x98.\n");

	quart_present = probe_quart();
	if (quart_present)
		kputs("QUART at 0xBB.\n");

	dma_present = !probe_z80dma();
	if (dma_present)
		kputs("Z80DMA detected at 0x04.\n");

	if (ds1302_present)
		kprintf("DS1302 detected at 0x%2x.\n", rtc_port);


	/* Devices in the C0-CF range cannot be used with Z180 */
	if (!z180_present) {
		i = 0xC0;
		while(i) {
			if (!ds1302_present || rtc_port != i) {
				if (m = probe_16x50(i))
					register_uart(i, &ns16x50_uart);
			}
			i += 0x08;
		}
	}
	display_uarts();
	/* FIXME: if we have no tms9918a and no CTC but a quart we should
	   use the quart as our timer */
}

void map_init(void)
{
}

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

static int strcmp(const char *d, const char *s)
{
	register char *s1 = (char *) d, *s2 = (char *) s, c1, c2;

	while ((c1 = *s1++) == (c2 = *s2++) && c1);
	return c1 - c2;
}

extern uint8_t nuart;

uint8_t platform_param(unsigned char *p)
{
	/* If we have a keyboard then the TMS9918A becomes a real tty
	   and we make it the primary console */
	if (strcmp(p, "zxkey") == 0 && !zxkey_present) {
		zxkey_present = 1;
		zxkey_init();
		if (tms9918a_present) {
			/* Add the consoles */
			uint8_t n = 0;
			shadowcon = 0;
			do {
				insert_uart(0x98, &tms_uart);
			} while(n < 4 && nuart <= NUM_DEV_TTY);
		}
		return 1;
	}
	return 0;
}


void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#ifdef CONFIG_PPIDE
	ppide_init();
#endif
#endif
#ifdef CONFIG_NET
	sock_init();
#endif
}
