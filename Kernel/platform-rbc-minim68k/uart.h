/* uart.h	types of UARTs		*/

#ifndef _UART_DOT_H
#define _UART_DOT_H


#ifdef	CONFIG_16x50

/* receive, interrupt enable, interrupt ident */
enum { UART_RBR=0, UART_IER, UART_IIR,			/* 0 1 2 */
			UART_LCR, UART_MCR, 		/* 3 4   */
			UART_LSR, UART_MSR, UART_SCR };	/* 5 6 7 */

/* transmit, ..., FIFO control */
enum { UART_THR=UART_RBR, UART_FCR=UART_IIR };

/* divisor latch LSB, MSB (low, high) accsessible when LCR:DLAB=1 */
enum { UART_DLL=0, UART_DLH };

/* IER bit definitions RcvBuf, TrnBuf, LinStat, ModmStat */
enum { B_ERBI=0, B_TBEI, B_ELSI, B_EDSSI,
	/* for 750 only: */	B_SLEEP, B_LOWPWR };

/* LCR bit definitions */
enum { B_WSL0=0, B_WSL1, B_STB, B_PEN, B_EPS, B_STICK, B_BRK1, B_DLAB };

/* MCR bit definitions */
enum { B_DTR=0, B_RTS, B_OUT1, B_OUT2, B_LOOP,   B_AFE };

/* LSR bit definitiona */
enum { B_DR=0, B_OE, B_PE, B_FE, B_BRE, B_THRE, B_TEMT, B_FIFO_ERR };

/* MSR bit definitions */
enum { B_DCTS=0, B_DDSR, B_TERI, B_DDCD, B_CTS, B_DSR, B_RI, B_DCD };

/* SCR bits are used on 750 class chips, otherwise, just a scratch register
   missing on 8250, but present on 8250A, ..450, ..550 & ..750 when special
   access code (0xBF) is not present in the LCR */


typedef enum { Unone=0, U8250, U16450, U16550, U16550A, U16550C, U16650, 
						U16750, U16754 } U16x50_t;
#define U8250A U16450

#if 0
const char *uart_name[] = {
	"none",
	"8250",
	"8250A/16450",
	"16550",
	"16550A",
	"16550C",
	"16650",
	"16750",
	"16754"
};
#endif

int uart_detect(uint8_t RBC_device_code);

#endif	/* end of configuration for the 8250, 16450, 16550(_,A,C), 16750 UART's */
#endif  /* end of _UART_DOT_H */
