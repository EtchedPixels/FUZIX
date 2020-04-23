#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <devsd.h>
#include <blkdev.h>
#include <ppide.h>
#include <rc2014.h>
#include <vt.h>
#include <netdev.h>
#include <zxkey.h>
#include <ps2bitbang.h>
#include <ps2kbd.h>
#include "vfd-term.h"
#include "z180_uart.h"

/* Everything in here is discarded after init starts */

static const uint8_t tmstext[] = {
	0x00,		/* m2:0 extvideo:0 */
	0xF0,		/* 16K, not blanked, int on, m1:1 m3:0 */
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
	} while(--ct && !(v & 0x80));

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

/* Look for a QUART at 0xBA */

#define QUARTREG(x)	(((x) << 11) | 0xBA)

#define MRA	0x00
#define CRA	0x02
#define IMR1	0x05
#define CRB	0x0A
#define IVR1	0x0C
#define CRC	0x12
#define ACR2	0x14
#define IMR2	0x15
#define CTU2	0x16
#define CTL2	0x17
#define CRD	0x1A
#define IVR2	0x1C
#define STCT2	0x1E	/* Read... */

static uint8_t probe_quart(void)
{
	uint8_t c = in16(QUARTREG(IVR1));

	c++;
	/* Make sure we they don't appear to affect one another */
	out16(QUARTREG(IVR1), c);

	if(in16(QUARTREG(IVR1)) != c)
		return 0;

	if(in16(QUARTREG(MRA)) == c)
		return 0;

	/* Ok now check IVR2 also works */
	out16(QUARTREG(IVR2), c + 1);
	if(in16(QUARTREG(IVR1)) != c)
		return 0;
	if(in16(QUARTREG(IVR2)) != c + 1)
		return 0;
	/* OK initialize things so we don't make a nasty mess when we
	   get going. We don't want interrupts for anything but receive
	   at this point. We can add line change later etc */
	out16(QUARTREG(IMR1), 0x22);
	out16(QUARTREG(IMR2), 0x22);
	/* Ensure active mode */
	out16(QUARTREG(CRA), 0xD0);
	/* Clocking */
	out16(QUARTREG(CRC), 0xD0);
	/* Reset the channels */
	out16(QUARTREG(CRA), 0x20);
	out16(QUARTREG(CRA), 0x30);
	out16(QUARTREG(CRA), 0x40);
	out16(QUARTREG(CRB), 0x20);
	out16(QUARTREG(CRB), 0x30);
	out16(QUARTREG(CRB), 0x40);
	out16(QUARTREG(CRC), 0x20);
	out16(QUARTREG(CRC), 0x30);
	out16(QUARTREG(CRC), 0x40);
	out16(QUARTREG(CRD), 0x20);
	out16(QUARTREG(CRD), 0x30);
	out16(QUARTREG(CRD), 0x40);
	/* We need to set ACR1/ACR2 once we do rts/cts and modem lines right */
	return 1;
}

/* Our counter counts clock/16 so 460800 clocks/second */

static void quart_clock(void)
{
	/* Timer, clock / 16 */
	out16(QUARTREG(ACR2), 0x70);	/* Adjust for RTS/CTS too */
	/* 10 ticks per second */
	out16(QUARTREG(CTL2), 11520 & 0xFF);
	out16(QUARTREG(CTU2), 11520 >> 8);
	/* Timer interrupt also wanted */
	out16(QUARTREG(IMR2), 0x22 | 0x08);
	/* Timer on */
	in16(QUARTREG(STCT2));
	/* Tell the quart driver to do do timer ticks */
	timer_source = TIMER_QUART;
	kputs("quart clock enabled\n");
}

__sfr __at 0xA0 sc26c92_mra;
__sfr __at 0xA2 sc26c92_cra;
__sfr __at 0xA4 sc26c92_acr;
__sfr __at 0xA6 sc26c92_ctu;
__sfr __at 0xA7 sc26c92_ctl;
__sfr __at 0xA5 sc26c92_imr;
__sfr __at 0xA8 sc26c92_mrb;
__sfr __at 0xAA sc26c92_crb;
__sfr __at 0xAE sc26c92_start;
__sfr __at 0xAF sc26c92_stop;

static uint8_t probe_sc26c92(void)
{
	volatile uint8_t dummy;

	/* These dummy reads for timer control reply FF */
	if (sc26c92_start != 0xFF || sc26c92_stop != 0xFF)
		return 0;

	sc26c92_acr = 0x30;		/* Count using the 7.37MHz clock */
	dummy = sc26c92_start;		/* Set it running */
	if (sc26c92_ctl == sc26c92_ctl)	/* Reads should show different */
		return 0;		/* values */
	dummy = sc26c92_stop;		/* Stop the clock */
	if (sc26c92_ctl != sc26c92_ctl)	/* and now same values */
		return 0;
	/* Ok looks like an SC26C92  */
	sc26c92_cra = 0x10;		/* MR1 always */
	sc26c92_cra = 0xB0;		/* MR0 on a 26C92, X bit control on a 88C681 */
	sc26c92_mra = 0x00;		/* We write MR1/MR2 or MR0/1... */
	sc26c92_mra = 0x01;
	sc26c92_cra = 0x10;		/* MR1 */
	sc26c92_imr = 0x22;
	if (sc26c92_mra & 0x01) {
		/* SC26C92 */
		sc26c92_crb = 0xB0;	/* Fix up MR0B, MR0A was done in the probe */
		sc26c92_mrb = 0x00;
		sc26c92_acr = 0x80;	/* ACR on counter off */
		return 1;
	}
	/* 88C681 */
	sc26c92_acr = 0x00;
	return 2;
}

static void sc26c92_timer(void)
{
	volatile uint8_t dummy;
	if (sc26c92_present == 1)		/* SC26C92 */
		sc26c92_acr = 0xB0;		/* Counter | ACR = 1 */
	else					/* 88C681 */
		sc26c92_acr = 0x30;		/* Counter | ACR = 0 */
	sc26c92_ctl = 46080 & 0xFF;
	sc26c92_ctu = 46080 >> 8;
	dummy = sc26c92_start;
	sc26c92_imr = 0x32;		/* Timer and both rx/tx */
	timer_source = TIMER_SC26C92;
}

void init_hardware_c(void)
{
#ifdef CONFIG_VFD_TERM
	vfd_term_init();
#endif
	ramsize = 512;
	procmem = 512 - 80;

	tms9918a_present = probe_tms9918a();
	if (tms9918a_present) {
		shadowcon = 1;
		timer_source = TIMER_TMS9918A;
	}

	/* FIXME: When ROMWBW handles second SIO, or Z180 as
	   console we will need to address this better */
	if (z180_present) {
		z180_setup(!ctc_present);
		register_uart(Z180_IO_BASE, &z180_uart0);
		register_uart(Z180_IO_BASE + 1, &z180_uart1);
		rtc_port = 0x0C;
		rtc_shadow = 0x0C;
		timer_source = TIMER_Z180;
	}

	/* Set the right console for kernel messages */
	/* ROMWBW favours the UART then SIO then ACIA */
	if (u16x50_present) {
		register_uart(0xA0, &ns16x50_uart);
		if (probe_16x50(0xA8))
			register_uart(0xA8, &ns16x50_uart);
	}
	if (sio_present) {
		register_uart(0x80, &sio_uart);
		register_uart(0x82, &sio_uartb);
	}
	if (acia_present)
		register_uart(0xA0, &acia_uart);
	/* TODO: sc26c92 as boot probe if added to ROMWBW */
}

__sfr __at 0xBC copro_ack;
__sfr __banked __at 0xFFBC copro_boot;	/* INT, NMI reset high */
__sfr __banked __at 0xBC copro_reset;	/* reset low */

static uint8_t probe_copro(void)
{
	uint8_t i = 0;
	uint8_t c;

	copro_reset = 0x00;		/* Force a reset */
	while(i < 255)
		i++;

	copro_boot = 0x00;

	i = 0;
	while(i < 255 && copro_ack != 0xAA) {
		i++;
	}
	if (i == 255)
		return 0;

	copro_boot = 0xFF;
	i = 0;
	while(i < 255 && copro_ack == 0xAA) {
		i++;
	}
	if (i == 255)
		return 0;
	/* Now read the banner and print it */
	while(1) {
		c = copro_ack;
		/* Ack the byte */
		copro_boot = 0x00;
		if (c == 0)
			break;
		/* While we print the char the coprocessor will get the next
		   one ready - and it will beat us to the next step */
		kputchar(c);
		c = copro_ack;
		copro_boot = 0x80;
		if (c == 0)
			break;
		kputchar(c);
	}
	kputchar('\n');
	return 1;
}

/*
 *	Do the main memory bank and device set up
 */
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

	quart_present = probe_quart();
	/* Further ports we register at this point */
	if (quart_present) {
		register_uart(0x00BA, &quart_uart);
		register_uart(0x40BA, &quart_uart);
		register_uart(0x80BA, &quart_uart);
		register_uart(0xC0BA, &quart_uart);
		/* If we don't have a TMS9918A then the QUART is the next
		   best clock choice */
		if (timer_source == TIMER_NONE)
			quart_clock();
	}

	if (sio1_present) {
		register_uart(0x84, &sio_uart);
		register_uart(0x86, &sio_uartb);
	}

	if (ctc_present) {
		if (timer_source == TIMER_NONE)
			timer_source = TIMER_CTC;
		else {
			/* Turn off our CTC interrupts */
			CTC_CH2 = 0x43;
			CTC_CH3 = 0x43;
		}
		kputs("Z80 CTC detected at 0x88.\n");
	}

	if (tms9918a_present) {
		kputs("TMS9918A VDP detected at 0x98.\n");
	}

	if (!acia_present)
		sc26c92_present = probe_sc26c92();

	if (sc26c92_present == 1) {
		kputs("SC26C92 detected at 0xA0.\n");
		register_uart(0x00A0, &sc26c92_uart);
		register_uart(0x00A8, &sc26c92_uart);
		if (timer_source == TIMER_NONE)
			sc26c92_timer();
	}
	if (sc26c92_present == 2) {
		kputs("XR88C681 detected at 0xA0.\n");
		register_uart(0x00A0, &xr88c681_uart);
		register_uart(0x00A8, &xr88c681_uart);
		if (timer_source == TIMER_NONE)
			sc26c92_timer();
	}

	/* Complete the timer set up */
	if (timer_source == TIMER_NONE)
		kputs("Warning: no timer available.\n");
	else
		platform_tick_present = 1;

	dma_present = !probe_z80dma();
	if (dma_present)
		kputs("Z80DMA detected at 0x04.\n");

	if (ds1302_present)
		kprintf("DS1302 detected at 0x%2x.\n", rtc_port);

	/* Boot the coprocessor if present (just one for now) */
	copro_present = probe_copro();
	if (copro_present)
		kputs("Z80 Co-processor at 0xBC\n");

	/* Normal RC2014 is 8 clocks/us or so. Allow more for faster
	   processors - we don't do much output bashing anyway. Should be
	   good to 25MHz */
	kbsave = 0x00;
	kbdelay = 0x0A14;	/* High bits are a djnz delay for 20us
				   Low a 44us djnz delay */
	/* Port default for the PS/2 card */
	kbport = 0xBB;
	/* 150uS in 38 clock loops : set for 8MHz */
	kbwait = 250;	/* Needs to be about 200us */
	ps2kbd_present = ps2kbd_init();
	if (ps2kbd_present) {
		kputs("PS/2 Keyboard at 0xBB\n");
		if (!zxkey_present && tms9918a_present) {
			/* Add the consoles */
			uint8_t n = 0;
			shadowcon = 0;
			kputs("Switching to video output.\n");
			do {
				insert_uart(0x98, &tms_uart);
				n++;
			} while(n < 4 && nuart <= NUM_DEV_TTY);
		}
	}
	if (ps2kbd_present & 2) {
		kputs("PS/2 Mouse at 0xBB\n");
		/* TODO: wire to input layer and interrupt */
	}

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

uint8_t platform_param(unsigned char *p)
{
	/* If we have a keyboard then the TMS9918A becomes a real tty
	   and we make it the primary console */
	if (strcmp(p, "zxkey") == 0 && !zxkey_present && !ps2kbd_present) {
		zxkey_present = 1;
		zxkey_init();
		if (tms9918a_present) {
			/* Add the consoles */
			uint8_t n = 0;
			shadowcon = 0;
			do {
				insert_uart(0x98, &tms_uart);
				n++;
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
#ifdef CONFIG_SD
	pio_setup();
	devsd_init();
#endif
#ifdef CONFIG_NET
	sock_init();
#endif
}
