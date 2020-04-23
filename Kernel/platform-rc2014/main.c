#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <devinput.h>
#include <rtc.h>
#include <ds1302.h>
#include <rc2014.h>
#include <ps2kbd.h>
#include <zxkey.h>

extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint8_t ctc_present;
uint8_t sio_present;
uint8_t sio1_present;
uint8_t z180_present;
uint8_t quart_present;
uint8_t tms9918a_present;
uint8_t dma_present;
uint8_t zxkey_present;
uint8_t copro_present;
uint8_t ps2kbd_present;
uint8_t ps2mouse_present;
uint8_t sc26c92_present;
uint8_t u16x50_present;

uint8_t platform_tick_present;
uint8_t timer_source = TIMER_NONE;

/* From ROMWBW */
uint16_t syscpu;
uint16_t syskhz;
uint8_t systype;

/* For RTC */
uint8_t rtc_shadow;
uint16_t rtc_port = 0x00C0;

/* TMS9918A */
uint8_t vtattr_cap;
uint16_t vdpport = 0x99 + (40 << 8);	/* 256 * width + port */

uint8_t shadowcon;

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

void platform_discard(void)
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

void platform_idle(void)
{
	if (timer_source != TIMER_NONE && !ps2kbd_present)
		__asm halt __endasm;
	else {
		irqflags_t irq = di();
		sync_clock();
		irqrestore(irq);
	}
}


void do_timer_interrupt(void)
{
	fd_tick();
	fd_tick();
	timer_interrupt();
}

static int16_t timerct;

/* Call timer_interrupt at 10Hz */
static void timer_tick(uint8_t n)
{
	timerct += n;
	while (timerct >= 20) {
		do_timer_interrupt();
		timerct -= 20;
	}
}

void platform_interrupt(void)
{
	/* FIXME: For Z180 we know if the ASCI ports are the source so
	   should fastpath them (vector 8 and 9) */
	uint8_t ti_r = 0;

	if (timer_source == TIMER_TMS9918A)
		ti_r = tms9918a_ctrl;

	tty_pollirq();

	/* On the Z180 we use the internal timer */
	if (timer_source == TIMER_Z180) {
		if (irqvector == 3)	/* Timer 0 */
			do_timer_interrupt();
	/* The TMS9918A is our second best choice as the CTC must be wired
	   right and may not be wired as we need it */
	} else if (ti_r & 0x80) {
		/* Running as a home computer not serial */
		if (zxkey_present)
			zxkey_poll();
		if (ps2kbd_present)
			ps2kbd_poll();
		/* We are using the TMS9918A as a timer */
		timerct++;
		if (timerct == 6) {	/* Always NTSC */
			do_timer_interrupt();
			timerct = 0;
		}
	/* If not and we have no QUART then pray the CTC works */
	} else if (timer_source == TIMER_CTC) {
		uint8_t n = 255 - CTC_CH3;
		CTC_CH3 = 0x47;
		CTC_CH3 = 255;
		timer_tick(n);
	}
}


/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

int strlen(const char *p)
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
