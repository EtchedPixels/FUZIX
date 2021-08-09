#ifndef __SERIAL_H
#define __SERIAL_H

void serial_early_init(void);
void serial_late_init(void);

/* From the manual: chapter 19 */
#define UART_BASE(n)	(0x4000C000 + (0x1000 * (n)))
#define UART_DR		0x00
#define UART_RSR	0x04
#define UART_ECR	0x04
#define UART_FR		0x18
#define 	UART_FR_TXFF	0x20
#define		UART_FR_RXFE	0x10
#define UART_ILPR	0x20
#define UART_IBRD	0x24
#define UART_FBRD	0x28
#define UART_LCRH	0x2C
#define		UART_LCRH_FEN	0x10
#define UART_CTL	0x30
#define		UART_CTL_UARTEN			0x0001
#define		UART_CTL_TXE			0x0100
#define		UART_CTL_RXE			0x0200
#define UART_IFLS	0x34
#define		UART_IFLS_TXIFLSEL_18TH		0x00
#define		UART_IFLS_RXIFLSEL_18TH		0x00
#define UART_IM		0x38
#define		UART_IM_RXIM	0x10
#define		UART_IM_RTIM	0x40
#define UART_MIS	0x40
#define		UART_MIS_RXMIS	0x10
#define 	UART_MIS_TXMIS	0x20
#define		UART_MIS_RTMIS	0x40
#define UART_ICR	0x44

#endif /* __SERIAL_H */
