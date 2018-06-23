#ifndef __RC2014_SIO_DOT_H__
#define __RC2014_SIO_DOT_H__

#include "config.h"

#define SIO0_IVT 8

#define SIO0_BASE 0x80
__sfr __at (SIO0_BASE + 0) SIOA_D;
__sfr __at (SIO0_BASE + 2) SIOA_C;
__sfr __at (SIO0_BASE + 1) SIOB_D;
__sfr __at (SIO0_BASE + 3) SIOB_C;

/* ACIA is at same address as SIO. Assume CONFIG_ACIA or CONFIG_SIO has been
   defined in config.h, but not both.
*/

#define ACIA_BASE 0x80
__sfr __at (ACIA_BASE + 0) ACIA_C;
__sfr __at (ACIA_BASE + 1) ACIA_D;

extern bool boot_from_rom;

#endif
