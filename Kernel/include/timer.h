#ifndef __TIMER_DOT_H__
#define __TIMER_DOT_H__

#include <stdbool.h>
#include <kernel.h>

typedef uint16_t timer_t;

extern timer_t system_timer; /* counts up at TICKSPERSEC, wraps to zero */
timer_t set_timer_duration(uint16_t duration); /* good for up to approx 0x8000 ticks */
bool timer_expired(timer_t timer_val);

#endif
