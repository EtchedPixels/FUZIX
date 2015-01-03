#ifndef __TIMER_DOT_H__
#define __TIMER_DOT_H__

#include <stdbool.h>
#include <kdata.h>
#include <kernel.h>

typedef uint16_t timer_t;

#define set_timer_ms(msec) (set_timer_duration(msec < 100U ? 1 : ((msec) / 100U)))
#define set_timer_sec(sec) (set_timer_duration((sec) * 10))
timer_t set_timer_duration(uint16_t duration); /* good for up to 32K ticks */
bool timer_expired(timer_t timer_val);

#endif
