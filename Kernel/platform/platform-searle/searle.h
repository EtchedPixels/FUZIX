#ifndef __TOM_DOT_H__
#define __TOM_DOT_H__

/* Needs generalizing and tidying up across the RC2014 systems */

#include "config.h"

/* SIO 2 ports */

#define SIO0_BASE 0x00
#define SIOA_D	(SIO0_BASE + 0)
#define SIOB_D	(SIO0_BASE + 1)
#define SIOA_C	(SIO0_BASE + 2)
#define SIOB_C	(SIO0_BASE + 3)

extern void sio2_otir(uint8_t port);

#endif
