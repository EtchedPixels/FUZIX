#ifndef __RCBUS_SIO_DOT_H__
#define __RCBUS_SIO_DOT_H__

/* Simple80 has somewhat differing addressing */

#include "config.h"

#define SIO0_IVT 8

#define SIO0_BASE 0x00
#define SIOA_D	(SIO0_BASE + 0)
#define SIOA_C	(SIO0_BASE + 1)
#define SIOB_D	(SIO0_BASE + 2)
#define SIOB_C	(SIO0_BASE + 3)

#define CTC_CH0	0xD0
#define CTC_CH1	0xD1
#define CTC_CH2	0xD2
#define CTC_CH3	0xD3

extern void sio2_otir(uint8_t port);

extern uint8_t ctc_present;

#endif
