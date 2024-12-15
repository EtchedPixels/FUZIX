#include <kernel.h>
#include <kdata.h>
#include <timer.h>
#include <printf.h>

/* WRS: simple timer functions

   These use a 16bit "miniticks" to keep the generated code nice
*/

timer_t set_timer_duration(uint16_t duration)
{
	timer_t a;
	if (duration & 0x8000) {
		kprintf("bad timer duration 0x%x\n", duration);
	}
	sync_clock();
	/* obvious code is return (miniticks+duration), however we have to do */
	/* it this longwinded way or sdcc doesn't load miniticks atomically */
	/* sdcc 3.4.0 bug: ordering affects result */
	a = duration;
	a += ticks.h.low;

	return a;
}

uint8_t timer_expired(timer_t timer_val)
{
	sync_clock();
	return timer_val < ticks.h.low;
}

/*-----------------------------------------------------------*/
/* Read date/time to System location from global variable    */
void rdtime(time_t *tloc)
{
        irqflags_t irq = di();
	sync_clock();
        memcpy(tloc, &tod, sizeof(tod));
	irqrestore(irq);
}

/* These need to go away for most uses by 2100 or so */
void rdtime32(uint32_t *tloc)
{
        irqflags_t irq = di();
	sync_clock();
        *tloc = tod.low;
	irqrestore(irq);
}

void wrtime(time_t *tloc)
{
        irqflags_t irq = di();
	sync_clock();
        memcpy(&tod, tloc, sizeof(tod));
	irqrestore(irq);
}

static uint8_t tod_deci;	/* 10ths of a second count */
static uint8_t rtcsec;		/* Second number we expect from the RTC */
#ifdef CONFIG_RTC
static uint8_t rtcsync;		/* Counter in 1/10ths until we sync with the RTC */
#endif

static void tick_clock(void)
{
	if (++tod_deci != 10)
		return;
        tod_deci = 0;
        if (!++tod.low)
		++tod.high;
	/* Track the seconds value we expect to see from an RTC */
	rtcsec++;
}


/*
 *	We use the seconds counter on the RTC as a time counter and lock our
 *	time progression to it. This avoids doing horrible piles of math to
 *	use the RTC itself and avoids problems with non Y2K devices.
 *
 *	We allow for multi-second leaps. On boxes with many of the directly
 *	interfaced floppy controllers we can reasonably expect to lose IRQ
 *	service for annoyingly long times.
 *
 *	The awkward case here is the no-clock case. In that situation
 *	we are running the clock from the RTC and we therefore don't need
 *	or want to do magic adjustments. A system using NO_CLOCK should
 *	make plt_rtc_secs return 255 if there is no timer interrupt
 *	available.
 */

void updatetod(void)
{
#ifdef CONFIG_RTC
	uint8_t rtcnew;
	int8_t slide;
#endif

	tick_clock();

#if defined(CONFIG_RTC) && defined(CONFIG_RTC_INTERVAL)
	/* on some platforms reading the RTC is expensive so we don't do it
	   every decisecond. */
	if(++rtcsync != CONFIG_RTC_INTERVAL)
		return;
	rtcsync = 0;
	rtcnew = plt_rtc_secs();		/* platform function */

	if (rtcnew == 255 || rtcnew == rtcsec)
		return;

	slide = rtcnew - rtcsec;	/* Seconds elapsed */
	rtcsec = rtcnew;
	/*
	 *	We assume a small negative warp in seconds is telling us
	 *	that the clock is running too fast and we should stall.
	 */
	if (slide == -1) {
		tod_deci--;		/* Wrapping here is fine */
		return;
	}
	if (slide < 0)
		slide += 60;		/* Seconds wrapped */
	tod.low += slide;
	if (tod.low < slide)		/* 32bit wrap ? */
		tod.high++;
#endif
}

#ifdef CONFIG_RTC_INTERVAL

void inittod(void)
{
	rtcsec = plt_rtc_secs();
	tod_deci = 0;
}

#endif				/* NO_RTC */

/*
 *	Tickless support logic.
 */

#ifdef CONFIG_NO_CLOCK

/*
 *	Logic for tickless system. If you have an RTC you can ignore this.
 */

static uint8_t newticks = 0xFF;
static uint8_t oldticks;

static uint8_t re_enter;

/*
 *	The OS core will invoke this routine when idle (via plt_idle) but
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
	if (!plt_tick_present) {
		irqflags_t irq = di();
		int16_t tmp;
		if (!re_enter++) {
			/* Get a rolling tick count so we can work out
			   how much time elapsed */
			oldticks = newticks;
			newticks = plt_rtc_secs();
			if (oldticks != 0xFF) {
				tmp = newticks - oldticks;
				if (tmp) {
					if (tmp < 0)
						tmp += 60;
					tmp *= 10;
					while(tmp--) {
						timer_interrupt();
					}
				}
			}
		}
		re_enter--;
		irqrestore(irq);
	}
}

#endif
