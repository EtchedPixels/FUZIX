#ifndef __LB_H__
#define __LB_H__

#include "config.h"

#define DART0_BASE 0x80
__sfr __at (DART0_BASE + 0x00) DARTA_D;
__sfr __at (DART0_BASE + 0x04) DARTA_C;
__sfr __at (DART0_BASE + 0x08) DARTB_D;
__sfr __at (DART0_BASE + 0x0C) DARTB_C;

__sfr __at 0x40 CTC_CH0;
__sfr __at 0x50 CTC_CH1;
__sfr __at 0x60 CTC_CH2;
__sfr __at 0x70 CTC_CH3;

__sfr __at 0x00 BCR;
#define BCR_FDC16	0x80
#define BCR_ROMOUT	0x40
#define BCR_SDEN	0x20
#define BCR_SIDE1	0x10
#define BCR_DS		0x0F

__sfr __at 0x01 LPDATA;
__sfr __at 0x02 LPSTON;
__sfr __at 0x03 LPSTROFF;

__sfr __at 0xC0 FD_WCR;
__sfr __at 0xC1 FD_WTR;
__sfr __at 0xC2 FD_WSR;
__sfr __at 0xC3 FD_WDR;
__sfr __at 0xC4 FD_RCR;
__sfr __at 0xC5 FD_RTR;
__sfr __at 0xC6 FD_RSR;
__sfr __at 0xC7 FD_RDR;

#define NCR5380_BASE	0x20

extern void dart_otir(uint8_t port) __z88dk_fastcall;

#endif
