#ifndef __Z80RETRO_DOT_H__
#define __Z80RETRO_DOT_H__

#include "config.h"

#define SIO0_BASE 0x80
__sfr __at (SIO0_BASE + 0) SIOB_D;
__sfr __at (SIO0_BASE + 1) SIOA_D;
__sfr __at (SIO0_BASE + 2) SIOB_C;
__sfr __at (SIO0_BASE + 3) SIOA_C;

__sfr __at 0x40 CTC_CH0;
__sfr __at 0x41 CTC_CH1;
__sfr __at 0x42 CTC_CH2;
__sfr __at 0x43 CTC_CH3;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;

extern uint8_t ds1302_present;

#endif
