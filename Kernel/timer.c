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
	/* obvious code is return (miniticks+duration), however we have to do */
	/* it this longwinded way or sdcc doesn't load miniticks atomically */
	/* sdcc 3.4.0 bug: ordering affects result */
	a = duration;
	a += ticks.h.low;

	return a;
}

bool timer_expired(timer_t timer_val)
{
	return ((timer_val - ticks.h.low) & 0x8000);
}

/*-----------------------------------------------------------*/
/* Read date/time to System location from global variable    */
void rdtime(time_t *tloc)
{
        irqflags_t irq = di();
        memcpy(tloc, &tod, sizeof(tod));
	irqrestore(irq);
}

/* These need to go away for most uses by 2038 */
void rdtime32(uint32_t *tloc)
{
        irqflags_t irq = di();
        *tloc = tod.low;
	irqrestore(irq);
}

void wrtime(time_t *tloc)
{
        irqflags_t irq = di();
        memcpy(&tod, tloc, sizeof(tod));
	irqrestore(irq);
}

#ifndef CONFIG_RTC
static uint8_t tod_deci;
/* Update software emulated clock. Called ten times
   a second */
void updatetod(void)
{
	if (++tod_deci != 10)
		return;
        tod_deci = 0;
        if (!++tod.low)
		++tod.high;
}
#else

static uint8_t rtcsec;
#ifdef CONFIG_RTC_INTERVAL
static uint8_t tod_deci;
#endif

/*
 *	We use the seconds counter on the RTC as a time counter and lock our
 *	time progression to it. This avoids doing horrible piles of math to
 *	use the RTC itself and avoids problems with non Y2K devices.
 *
 *	We allow for multi-second leaps. On boxes with many of the directly
 *	interfaced floppy controllers we can reasonably expect to lose IRQ
 *	service for annoyingly long times.
 */
void updatetod(void)
{
	uint8_t rtcnew;
	int8_t slide;

#ifdef CONFIG_RTC_INTERVAL
	/* on some platforms reading the RTC is expensive so we don't do it
	   every decisecond. */
	if(++tod_deci != CONFIG_RTC_INTERVAL){
            if(!(tod_deci % 10)){ /* we still need to count each second */
                rtcsec++;
                slide=1;
                goto addtod;
            }
	    return;
        }
	tod_deci = 0;
#endif

	rtcnew = platform_rtc_secs();		/* platform function */

	if (rtcnew == rtcsec)
		return;
	slide = rtcnew - rtcsec;	/* Seconds elapsed */
	rtcsec = rtcnew;
	/*
	 *	FIXME: it would be nice if we see a backwards slide of 1 or 2
	 *	to stall the OS clock so we can use an rtc to track a not so
	 *	stable system clock.
	 */
addtod:
	if (slide < 0)
		slide += 60;		/* Seconds wrapped */
	tod.low += slide;
	if (tod.low < slide)		/* 32bit wrap ? */
		tod.high++;
}

void inittod(void)
{
	rtcsec = platform_rtc_secs();
}

#endif				/* NO_RTC */
