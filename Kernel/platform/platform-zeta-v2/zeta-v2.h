#ifndef __ZETA_V2_DOT_H__
#define __ZETA_V2_DOT_H__

#include "config.h"

#define UART0_BASE 0x68
#define UART0_RBR (UART0_BASE + 0)	/* receive buffer register, R/O */
#define UART0_THR (UART0_BASE + 0)	/* xmit holding register, W/O */
#define UART0_IER (UART0_BASE + 1)	/* interrupt enable register */
#define UART0_IIR (UART0_BASE + 2)	/* interrupt ident. register, R/O */
#define UART0_FCR (UART0_BASE + 2)	/* FIFO control register, W/O */
#define UART0_LCR (UART0_BASE + 3)	/* Line control register */
#define UART0_MCR (UART0_BASE + 4)	/* Modem control register */
#define UART0_LSR (UART0_BASE + 5)	/* Line status register */
#define UART0_MSR (UART0_BASE + 6)	/* Modem status register */
#define UART0_SCR (UART0_BASE + 7)	/* Scratch register */
#define UART0_DLL (UART0_BASE + 0)	/* Divisor latch - low byte */
#define UART0_DLH (UART0_BASE + 1)	/* Divisor latch - high byte */

extern bool boot_from_rom;

#endif
