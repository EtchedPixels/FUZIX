#include <kernel.h>
#include <tty.h>
#include "config.h"
#include "cpu.h"
#include "tm4c129x.h"
#include "gpio.h"
#include "interrupt.h"
#include "serial.h"

static unsigned char tbuf1[TTYSIZ];

uint32_t last_im = 0U;

struct s_queue ttyinq[NUM_DEV_TTY + 1U] = {
  { .q_base =   NULL,
    .q_head =   NULL,
    .q_tail =   NULL,
    .q_size =   0,
    .q_count =  0,
    .q_wakeup = 0 },
  { .q_base =   tbuf1,
    .q_head =   tbuf1,
    .q_tail =   tbuf1,
    .q_size =   TTYSIZ,
    .q_count =  0,
    .q_wakeup = (TTYSIZ / 2) }
};

tcflag_t termios_mask[NUM_DEV_TTY + 1U] = {
  0U, _CSYS,
};

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
  uint32_t im = 0U;

  serial_disableuartint(minor - 1U, &im);
  serial_waittxnotfull(minor - 1U);
  serial_out(minor - 1U, UART_DR, c);
  serial_waittxnotfull(minor - 1U);
  serial_restoreuartint(minor - 1U, im);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
  return serial_txnotfull(minor - 1U) ? TTY_READY_NOW : TTY_READY_SOON;
}

int tty_carrier(uint_fast8_t minor)
{
  return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_sleeping(uint_fast8_t minor)
{
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
  if (c == '\n')
    kputchar('\r');
  tty_putc(1U, c);
}

static irqreturn_t serial_isr(unsigned int irq, void *dev_id, uint32_t *regs)
{
  uint32_t c, mis;
  unsigned u;
  unsigned minor = (unsigned)dev_id;

  for (u = 0U; u < 256U; u++) {
    mis = serial_in(minor - 1U, UART_MIS);
    serial_out(minor - 1U, UART_ICR, mis);
    if (mis & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
      while (serial_rxavailable(minor - 1U)) {
        c = ((serial_in(minor - 1U, UART_DR)) & 0xffU);
        tty_inproc(minor, ((uint8_t)(c)));
      }
      continue;
    }
    if (mis & UART_MIS_TXMIS) {
      tty_outproc(minor);
      continue;
    }
    break;
  }
  return IRQ_HANDLED;
}

void serial_late_init(void)
{
  uint32_t u;
  const unsigned minor = minor(BOOT_TTY);

  serial_out(minor - 1U, UART_IFLS,
             UART_IFLS_TXIFLSEL_18TH | UART_IFLS_RXIFLSEL_18TH);
  serial_out(minor - 1U, UART_IM, UART_IM_RXIM | UART_IM_RTIM);
  u = serial_in(minor - 1U, UART_LCRH);
  u |= UART_LCRH_FEN;
  serial_out(minor - 1U, UART_LCRH, u);
  u = serial_in(minor - 1U, UART_CTL);
  u |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
  serial_out(minor - 1U, UART_CTL, u);
  last_im = serial_in(minor - 1U, UART_IM);
  request_irq(IRQ_UART0, serial_isr, (void *)(minor));
}

#define SYSCON_RCGUART	0x400FE618
#define SYSCON_PCUART	0x400FE918
#define SYSCON_PRUART	0x400FEA18

void serial_early_init(void)
{
  uint32_t ctl;
  const unsigned minor = minor(BOOT_TTY);

  /* Power up the UART */
  tm4c129x_modreg(SYSCON_PCUART, 0, 1U);
  tm4c129x_modreg(SYSCON_RCGUART, 0, 1U);

  /* Put the console on the right pins and power them up */
  gpio_setup_pin(GPIO_PORT('A'), GPIO_PINFUN_PFIO, 0U, GPIO_PAD_STD, 0U, 1U, 0U);
  gpio_setup_pin(GPIO_PORT('A'), GPIO_PINFUN_PFIO, 1U, GPIO_PAD_STD, 0U, 1U, 0U);

  /* Can't use the UART until it finishes coming out of power gating */
  while (!(inl(SYSCON_PRUART) & 1));

  /* Disable the UART by clearing the UARTEN bit in the UART CTL register */
  ctl = inl(UART_BASE(0) + UART_CTL);
  ctl &= ~UART_CTL_UARTEN;
  serial_out(minor - 1U, UART_CTL, ctl);

  /* We need 115200 baud, and 16 samples per bit (like a 16x50) and we run
     at 120MHz */
  serial_out(minor - 1U, UART_IBRD, 65U);
  serial_out(minor - 1U, UART_FBRD, 192000U);

  /* 8N1 FIFO on */
  serial_out(minor - 1U, UART_LCRH, (3U << 5U) | 0x10U);

  ctl |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
  serial_out(minor - 1U, UART_CTL, ctl);
}
