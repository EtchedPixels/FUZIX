#ifndef __RC2014_SIO_DOT_H__
#define __RC2014_SIO_DOT_H__

#include "config.h"

#define SIO0_IVT 8

/* Standard RC2014 */
#define SIO0_BASE 0x80
__sfr __at (SIO0_BASE + 0) SIOA_C;
__sfr __at (SIO0_BASE + 1) SIOA_D;
__sfr __at (SIO0_BASE + 2) SIOB_C;
__sfr __at (SIO0_BASE + 3) SIOB_D;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;
extern void cpld_bitbang(uint8_t c) __z88dk_fastcall;

#endif
