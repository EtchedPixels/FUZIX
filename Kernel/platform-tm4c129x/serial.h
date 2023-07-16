#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>
#include <stdbool.h>

#include "cpu.h"

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

extern uint32_t last_im;

static inline uint32_t serial_in(unsigned dev, int offset)
{
  return inl(UART_BASE(dev) + offset);
}

static inline void serial_out(unsigned dev, int offset, uint32_t value)
{
  outl(UART_BASE(dev) + offset, value);
}

static inline void serial_disableuartint(unsigned dev, uint32_t *im)
{
  if (im)
    *im = last_im;
  last_im = 0U;
  serial_out(dev, UART_IM, 0U);
}

static inline void serial_restoreuartint(unsigned dev, uint32_t im)
{
  last_im = im;
  serial_out(dev, UART_IM, im);
}

static inline bool serial_rxavailable(unsigned dev)
{
  return ((serial_in(dev, UART_FR)) & UART_FR_RXFE) == 0U;
}

static inline bool serial_txnotfull(unsigned dev)
{
  return ((serial_in(dev, UART_FR)) & UART_FR_TXFF) == 0U;
}

static inline void serial_waittxnotfull(unsigned dev)
{
  volatile unsigned u;

  for (u = 0U; u < 1000U; u++) {
    if (serial_txnotfull(dev))
      break;
  }
}

#endif /* __SERIAL_H */
