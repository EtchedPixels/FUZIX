#ifndef __RC2014_SIO_DOT_H__
#define __RC2014_SIO_DOT_H__

#include "config.h"

/* Standard RC2014 */
#define SIO0_BASE 0x80
#define SIOA_C	(SIO0_BASE + 0)
#define SIOA_D	(SIO0_BASE + 1)
#define SIOB_C	(SIO0_BASE + 2)
#define SIOB_D	(SIO0_BASE + 3)

/* ACIA is at same address as SIO but we autodetect */

#define ACIA_BASE 0x80
#define ACIA_C	(ACIA_BASE + 0)
#define ACIA_D	(ACIA_BASE + 1)

#define CTC_CH0	0x88
#define CTC_CH1	0x89
#define CTC_CH2	0x8A
#define CTC_CH3	0x8B

extern void sio2_otir(uint8_t port);

extern uint8_t acia_present;
extern uint8_t ctc_present;
extern uint8_t sio_present;
extern uint8_t sio1_present;

#endif
