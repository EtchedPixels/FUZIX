#ifndef __TIMER_DOT_H__
#define __TIMER_DOT_H__

#include <stdbool.h>
#include <kdata.h>
#include <kernel.h>

typedef uint16_t timer_t;

#define set_timer_ms(msec) (set_timer_duration(msec < 100U ? 1 : ((msec) / 100U)))
#define set_timer_sec(sec) (set_timer_duration((sec) * 10))
timer_t set_timer_duration(uint16_t duration); /* good for up to 32K ticks */
uint8_t timer_expired(timer_t timer_val);

#ifndef CONFIG_NO_CLOCK
#define sync_clock() 	do {} while(0)
#define update_sync_clock() do {} while(0)
#else
/* On a tickless system read the clock and run timer_interrupt the right number
   of times. Needs to di() and also just return if recursively called. */
extern void sync_clock(void);
/* On a tickless system notify the clock logic that the underlying time just
   moved. Called under di such that we sync_clock, move the clock, then
   call update_sync_clock */
extern void update_sync_clock(void);
#endif

#endif
