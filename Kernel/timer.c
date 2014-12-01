#include <kernel.h>
#include <kdata.h>
#include <timer.h>
#include <printf.h>

/* the interrupt handler increments this after every timer interrupt */
uint16_t system_timer;

/* WRS: timer functions */
timer_t set_timer_duration(uint16_t duration)
{
	if (duration & 0x8000) {
		kprintf("bad timer duration 0x%x\n", duration);
	}
	return (system_timer + duration);
}

bool timer_expired(timer_t timer_val)
{
	if ((timer_val - system_timer) > 0x7fff)
		return true;
	return false;
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
#ifdef CONFIG_RTC
	machine_set_clock(tloc);
#endif
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
#endif				/* NO_RTC */
