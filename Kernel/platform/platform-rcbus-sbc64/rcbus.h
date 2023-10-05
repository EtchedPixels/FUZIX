#ifndef __RCBUS_DOT_H__
#define __RCBUS_DOT_H__

#include "config.h"

#define SIO0_IVT 8

/* Standard RC2014 */
#define SIO0_BASE 0x80
__sfr __at (SIO0_BASE + 0) SIOA_C;
__sfr __at (SIO0_BASE + 1) SIOA_D;
__sfr __at (SIO0_BASE + 2) SIOB_C;
__sfr __at (SIO0_BASE + 3) SIOB_D;

#define SIO1_BASE 0x84
__sfr __at (SIO1_BASE + 0) SIOC_C;
__sfr __at (SIO1_BASE + 1) SIOC_D;
__sfr __at (SIO1_BASE + 2) SIOD_C;
__sfr __at (SIO1_BASE + 3) SIOD_D;

__sfr __at 0x88 CTC_CH0;
__sfr __at 0x89 CTC_CH1;
__sfr __at 0x8A CTC_CH2;
__sfr __at 0x8B CTC_CH3;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;
extern void cpld_bitbang(uint8_t c) __z88dk_fastcall;

extern uint8_t ctc_present;
extern uint8_t sio_present;
extern uint8_t sio1_present;

#endif
