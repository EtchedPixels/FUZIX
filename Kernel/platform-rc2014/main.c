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
#include <zxkey.h>

extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint8_t ctc_present;
uint8_t sio_present;
uint8_t sio1_present;
uint8_t z180_present;
uint8_t tms9918a_present;
uint8_t dma_present;
uint8_t zxkey_present;

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
	if (ctc_present || z180_present)
		__asm halt __endasm;
	else {
		irqflags_t irq = di();
		sync_clock();
		irqrestore(irq);
	}
}

static int16_t timerct;

/* Call timer_interrupt at 10Hz */
static void timer_tick(uint8_t n)
{
	timerct += n;
	while (timerct >= 20) {
		timer_interrupt();
		timerct -= 20;
	}
}

void platform_interrupt(void)
{
	/* FIXME: For Z180 we know if the ASCI ports are the source so
	   should fastpath them (vector 8 and 9) */
	uint8_t ti_r = 0;

	if (tms9918a_present)
		ti_r = tms9918a_ctrl;

	tty_pollirq();
	if ((ti_r & 0x80) && zxkey_present)
		zxkey_poll();

	if (z180_present) {
		if (irqvector == 3)	/* Timer 0 */
			timer_interrupt();
	} else if (ctc_present) {
		uint8_t n = 255 - CTC_CH3;
		CTC_CH3 = 0x47;
		CTC_CH3 = 255;
		timer_tick(n);
	} else if (ti_r & 0x80) {
		/* We areusing the TMS9918A as a timer */
		timerct++;
		if (timerct == 6) {	/* Always NTSC */
			timer_interrupt();
			timerct = 0;
		}
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
	/* For now */
}

/*
 *	Logic for tickless system. If you have an RTC you can ignore this.
 */

static uint8_t newticks = 0xFF;
static uint8_t oldticks;

static uint8_t re_enter;

/*
 *	Hardware specific logic to get the seconds. We really ought to enhance
 *	this to check minutes as well just in case something gets stuck for
 *	ages.
 */
static void sync_clock_read(void)
{
	uint8_t s;
	oldticks = newticks;
	ds1302_read_clock(&s, 1);
	s = (s & 0x0F) + (((s & 0xF0) >> 4) * 10);
	newticks = s;
}

/*
 *	The OS core will invoke this routine when idle (via platform_idle) but
 *	also after a system call and in certain other spots to ensure the clock
 *	is roughly valid. It may be called from interrupts, without interrupts
 *	or even recursively so it must protect itself using the framework
 *	below.
 *
 *	Having worked out how much time has passed in 1/10ths of a second it
 *	performs that may timer_interrupt events in order to advance the clock.
 *	The core kernel logic ensures that we won't do anything silly from a
 *	jump forward of many seconds.
 *
 *	We also choose to poll the ttys here so the user has some chance of
 *	getting control back on a messed up process.
 */
void sync_clock(void)
{
	if (!ctc_present && !tms9918a_present) {
		irqflags_t irq = di();
		int16_t tmp;
		if (!re_enter++) {
			sync_clock_read();
			if (oldticks != 0xFF) {
				tmp = newticks - oldticks;
				if (tmp < 0)
					tmp += 60;
				tmp *= 10;
				while(tmp--) {
					timer_interrupt();
				}
			}
		}
		re_enter--;
		irqrestore(irq);
	}
}

/*
 *	This method is called if the kernel has changed the system clock. We
 *	don't work out how much work we need to do by using it as a reference
 *	so we don't care.
 */
void update_sync_clock(void)
{
}
