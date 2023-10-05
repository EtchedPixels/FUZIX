#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <devinput.h>
#include <rtc.h>
#include <ds1302.h>
#include <ds12885.h>
#include <rcbus.h>
#include <ps2kbd.h>
#include <zxkey.h>
#include <softzx81.h>
#include <net_w5x00.h>

extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint8_t ctc_port;
uint8_t kio_port;		/* 0 means none */

uint8_t sio_present;
uint8_t sio1_present;
uint8_t z180_present;
uint8_t quart_present;
uint8_t tms9918a_present;
uint8_t ef9345_present;
uint8_t dma_present;
uint8_t zxkey_present;
uint8_t copro_present;
uint8_t ps2kbd_present;
uint8_t ps2mouse_present;
uint8_t sc26c92_present;
uint8_t u16x50_present;
uint8_t z512_present = 1;	/* We assume so and turn it off if not */
uint8_t fpu_present;
uint8_t eipc_present;
uint8_t macca_present;

uint8_t plt_tick_present;
uint8_t timer_source = TIMER_NONE;

uint8_t ticksperclk;

/* From ROMWBW */
uint16_t syscpu;
uint16_t syskhz;
uint8_t systype;
uint8_t romver;

/* For RTC */
uint8_t rtc_shadow;
#ifdef CONFIG_RC2014_EXTREME
uint16_t rtc_port = 0xC0B8;
#else
uint16_t rtc_port = 0x00C0;
#endif

/* TMS9918A */
uint8_t vtattr_cap;
uint16_t vdpport = 0x99 + (40 << 8);	/* 256 * width + port */

uint8_t shadowcon;
uint8_t soft81_on;

/* Our pool ends at 0x4000 */
uint8_t *initptr = (uint8_t *)0x4000;
uint8_t *code1ptr = (uint8_t *)0xC000;

struct blkbuf *bufpool_end = bufpool + NBUFS;

uint8_t *init_alloc(uint16_t size)
{
	uint8_t *p = initptr - size;
	if(p < (uint8_t *)bufpool_end)
		panic("imem");
	initptr = p;
	return p;
}

uint8_t *code1_alloc(uint16_t size)
{
	uint8_t *p = code1ptr - size;
	if(p < (uint8_t *)code1_end())
		panic("1mem");
	code1ptr = p;
	return p;
}

void plt_discard(void)
{
	uint16_t discard_size = (uint16_t)initptr - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

void plt_idle(void)
{
	if (timer_source != TIMER_NONE)
		__asm halt __endasm;
	else {
		irqflags_t irq = di();
		sync_clock();
#ifdef CONFIG_NET_W5100
		w5x00_poll();
#endif
#ifdef CONFIG_NET_W5300
		w5300_poll();
#endif
		irqrestore(irq);
	}
}


void do_timer_interrupt(void)
{
	fd_tick();
	fd_tick();
	timer_interrupt();
#ifdef CONFIG_NET_W5100
	w5x00_poll();
#endif
#ifdef CONFIG_NET_W5300
	w5300_poll();
#endif
}

static int16_t timerct;

/* Call timer_interrupt at 10Hz */
static void timer_tick(uint8_t n)
{
	timerct += n;
	while (timerct >= ticksperclk) {
		do_timer_interrupt();
		timerct -= ticksperclk;
	}
}

__sfr __at (Z180_IO_BASE + 0x10) TIME_TCR;      /* Timer control register                     */
__sfr __at (Z180_IO_BASE + 0x0C) TIME_TMDR0L;   /* Timer data register,    channel 0L         */

static void key_poll(void)
{
	/* Running as a home computer not serial */
	if (zxkey_present)
		zxkey_poll();
	if (ps2kbd_present && ps2_type == PS2_BITBANG) {
		if (!ps2busy) {
			int16_t n = ps2kbd_get();
			if (n >= 0)
				ps2kbd_byte(n);
		}
	}
}

void plt_interrupt(void)
{
	/* FIXME: For Z180 we know if the ASCI ports are the source so
	   should fastpath them (vector 8 and 9) */
	uint8_t ti_r = 0;


	/* We must never read this from interrupt unless it is our timer */
	if (timer_source == TIMER_TMS9918A) {
		ti_r = tms9918a_ctrl;
		if (ti_r & 0x80)
			wakeup(&shadowcon);
	}

	tty_pollirq();

	/* On the Z180 we use the internal timer */
	if (timer_source == TIMER_Z180) {
		if (irqvector == 3) {	/* Timer 0 */
			uint8_t r;
			r = TIME_TCR;
			r = TIME_TMDR0L;
			timerct++;
			if (timerct == 4) {
				do_timer_interrupt();
				timerct = 0;
			}
			/* We poll the bitbanger 40 times a second as it's slow */
			key_poll();
		}
	/* The TMS9918A is our second best choice as the CTC must be wired
	   right and may not be wired as we need it */
	} else if (ti_r & 0x80) {
		key_poll();
		/* We are using the TMS9918A as a timer */
		timerct++;
		if (timerct == 6) {	/* Always NTSC */
			do_timer_interrupt();
			timerct = 0;
		}
#ifdef CONFIG_RTC_DS12885
	/* Otherwise use DS12885 RTC if present */
	} else if (timer_source == TIMER_DS12885) {
		/* If IRQF and PF flags are set, interrupt is from 64 Hz signal */
		if (ds12885_interrupt() & (IRQF+PF)) {
			timerct++;
			/* 7 ticks = 109 ms; 6 ticks = 93.7 ms;
			 * (7+6+6+7+6) * 64 Hz = 500.0 ms */
			switch (timerct) {
				case 7: case 13: case 19: case 26:
					do_timer_interrupt();
					break;
				case 32:
					do_timer_interrupt();
					timerct = 0;
					break;
			}
		}
#endif
	/* If not and we have no QUART then pray the CTC works */
	} else if (timer_source == TIMER_CTC) {
		uint8_t n = 255 - in(ctc_port + 3);
		out(ctc_port + 3, 0x47);
		out(ctc_port +3, 0xFF);
		timer_tick(n);
		key_poll();
	}
	/* Poll the hardware PS/2 every interrupt as it may be the actual source */
	ps2_int();
	/* Soft ZX81 helper */
	if (soft81_on)
		softzx81_int();
}


/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

size_t strlen(const char *p)
{
	int len = 0;
	while(*p++)
		len++;
	return len;
}

void do_beep(void)
{
	if (ps2kbd_present)
		ps2kbd_beep();
}

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

int strcmp(const char *d, const char *s)
{
	register char *s1 = (char *) d, *s2 = (char *) s, c1, c2;

	while ((c1 = *s1++) == (c2 = *s2++) && c1);
	return c1 - c2;
}
