#ifndef __RC2014_SIO_DOT_H__
#define __RC2014_SIO_DOT_H__

#include "config.h"

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

/* ACIA is at same address as SIO but we autodetect */

#define ACIA_BASE 0x80
__sfr __at (ACIA_BASE + 0) ACIA_C;
__sfr __at (ACIA_BASE + 1) ACIA_D;

__sfr __at 0x88 CTC_CH0;
__sfr __at 0x89 CTC_CH1;
__sfr __at 0x8A CTC_CH2;
__sfr __at 0x8B CTC_CH3;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;

extern uint8_t acia_present;
extern uint8_t ctc_present;
extern uint8_t sio_present;
extern uint8_t sio1_present;
extern uint8_t z180_present;
extern uint8_t tms9918a_present;

#define UART_ACIA	1
#define UART_SIO	2
#define UART_Z180	3
#define UART_8250	4
#define UART_16450	5
#define UART_16550	6
#define UART_16550A	7
#define UART_16750	8

/* From ROMWBW */
extern uint16_t syscpu;
extern uint16_t syskhz;
extern uint8_t systype;

extern const char *uart_name[];

struct uart {
    uint8_t (*intr)(uint_fast8_t , uint_fast8_t);
    uint8_t (*writeready)(uint_fast8_t, uint_fast8_t);
    void (*putc)(uint_fast8_t, uint_fast8_t, uint_fast8_t);
    void (*setup)(uint_fast8_t, uint_fast8_t);
    uint8_t (*carrier)(uint_fast8_t, uint_fast8_t);
    uint16_t cmask;
};

extern struct uart *uart[NUM_DEV_TTY + 1];
extern uint8_t ttyport[NUM_DEV_TTY + 1];
extern uint8_t register_uart(uint8_t type, uint8_t port, struct uart *);

extern struct uart acia_uart;
extern struct uart sio_uart;
extern struct uart sio_uartb;
extern struct uart ns16x50_uart;
extern struct uart z180_uart0;
extern struct uart z180_uart1;

extern uint8_t *init_alloc(uint16_t size);

#endif
