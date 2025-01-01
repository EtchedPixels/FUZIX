#ifndef __ZETA_V2_DOT_H__
#define __ZETA_V2_DOT_H__

#include "config.h"

#define UART0_BASE 0x68
__sfr __at (UART0_BASE + 0) UART0_RBR;	/* receive buffer register, R/O */
__sfr __at (UART0_BASE + 0) UART0_THR;	/* xmit holding register, W/O */
__sfr __at (UART0_BASE + 1) UART0_IER;  /* interrupt enable register */
__sfr __at (UART0_BASE + 2) UART0_IIR;	/* interrupt ident. register, R/O */
__sfr __at (UART0_BASE + 2) UART0_FCR;	/* FIFO control register, W/O */
__sfr __at (UART0_BASE + 3) UART0_LCR;	/* Line control register */
__sfr __at (UART0_BASE + 4) UART0_MCR;	/* Modem control register */
__sfr __at (UART0_BASE + 5) UART0_LSR;	/* Line status register */
__sfr __at (UART0_BASE + 6) UART0_MSR;	/* Modem status register */
__sfr __at (UART0_BASE + 7) UART0_SCR;	/* Scratch register */
__sfr __at (UART0_BASE + 0) UART0_DLL;	/* Divisor latch - low byte */
__sfr __at (UART0_BASE + 1) UART0_DLH;	/* Divisor latch - high byte */

extern bool boot_from_rom;

#endif
