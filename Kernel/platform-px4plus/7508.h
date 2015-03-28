#ifndef _7508_H
#define _7508_H

extern void c7508_power_off(void);
extern void c7508_interrupt(void);
extern void c7508_msg(uint8_t *buf);

extern uint8_t rtc_clock;		/* Rolling 1 second timer */

#endif