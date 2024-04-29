#ifndef __Z80RETRO_DOT_H__
#define __Z80RETRO_DOT_H__

#include "config.h"

#define SIO0_BASE 0x80
#define SIOB_D	(SIO0_BASE + 0)
#define SIOA_D  (SIO0_BASE + 1)
#define SIOB_C	(SIO0_BASE + 2)
#define SIOA_C	(SIO0_BASE + 3)

#define CTC_CH0	0x40
#define CTC_CH1	0x41
#define CTC_CH2	0x42
#define CTC_CH3	0x43

extern void sio2_otir(uint8_t port);

extern uint8_t ds1302_present;

#endif
