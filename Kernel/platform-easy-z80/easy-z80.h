#ifndef __RC2014_SIO_DOT_H__
#define __RC2014_SIO_DOT_H__

#include "config.h"

/* We have a weird mismatch setup with easy-z80 */
#define SIO0_BASE 0x80
__sfr __at (SIO0_BASE + 0) SIOA_D;
__sfr __at (SIO0_BASE + 1) SIOA_C;
__sfr __at (SIO0_BASE + 2) SIOB_D;
__sfr __at (SIO0_BASE + 3) SIOB_C;

__sfr __at 0x88 CTC_CH0;
__sfr __at 0x89 CTC_CH1;
__sfr __at 0x8A CTC_CH2;
__sfr __at 0x8B CTC_CH3;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;

extern uint8_t ds1302_present;

#endif
