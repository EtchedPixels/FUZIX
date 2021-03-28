#include <stdint.h>
#include <stdbool.h>

#include <hardware/tiva_memorymap.h>
#include <hardware/tiva_uart.h>
#include <arch/tiva/tm4c_irq.h>

#include <kernel.h>
#include <tty.h>

#include "cpu.h"

#include "exceptions.h"

#include "config.h"

#include "serial.h"

static uint32_t last_im = 0U;

static inline uint32_t serial_in(int offset)
{
  return getreg32(TIVA_UART0_BASE + offset);
}

static inline void serial_out(int offset, uint32_t value)
{
  putreg32(value, TIVA_UART0_BASE + offset);
}

static inline void serial_disableuartint(uint32_t *im)
{
  if (im)
    *im = last_im;
  last_im = 0U;
  serial_out(TIVA_UART_IM_OFFSET, 0U);
}

static inline void serial_restoreuartint(uint32_t im)
{
  last_im = im;
  serial_out(TIVA_UART_IM_OFFSET, im);
}

bool serial_rxavailable(void)
{
  return ((serial_in(TIVA_UART_FR_OFFSET)) & UART_FR_RXFE) == 0U;
}

bool serial_txnotfull(void)
{
  return ((serial_in(TIVA_UART_FR_OFFSET)) & UART_FR_TXFF) == 0U;
}

static inline void serial_waittxnotfull(void)
{
  volatile unsigned u;

  for (u = 0U; u < 1000U; u++) {
    if (serial_txnotfull())
      break;
  }
}

void serial_putc(uint32_t c)
{
  uint32_t im = 0U;

  serial_disableuartint(&im);
  serial_waittxnotfull();
  serial_out(TIVA_UART_DR_OFFSET, c);
  serial_waittxnotfull();
  serial_restoreuartint(im);
}

static uint32_t *serial_isr(uint32_t irq, uint32_t *regs)
{
  uint32_t c, mis;
  unsigned u;

  for (u = 0U; u < 256U; u++) {
    mis = serial_in(TIVA_UART_MIS_OFFSET);
    serial_out(TIVA_UART_ICR_OFFSET, mis);
    if (mis & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
      while (serial_rxavailable()) {
        c = ((serial_in(TIVA_UART_DR_OFFSET)) & 0xffU);
        tty_inproc(minor(BOOT_TTY), ((uint8_t)(c)));
      }
      continue;
    }
    if (mis & UART_MIS_TXMIS) {
      tty_outproc(minor(BOOT_TTY));
      continue;
    }
    break;
  }
  return regs;
}

void serial_late_init(void)
{
  uint32_t u;

  serial_out(TIVA_UART_IFLS_OFFSET,
             UART_IFLS_TXIFLSEL_18TH | UART_IFLS_RXIFLSEL_18TH);
  serial_out(TIVA_UART_IM_OFFSET, UART_IM_RXIM | UART_IM_RTIM);
  u = serial_in(TIVA_UART_LCRH_OFFSET);
  u |= UART_LCRH_FEN;
  serial_out(TIVA_UART_LCRH_OFFSET, u);
  u = serial_in(TIVA_UART_CTL_OFFSET);
  u |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
  serial_out(TIVA_UART_CTL_OFFSET, u);
  last_im = serial_in(TIVA_UART_IM_OFFSET);
  exception_attach_handler(TIVA_IRQ_UART0, serial_isr);
  exception_enable(TIVA_IRQ_UART0);
}
