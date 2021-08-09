#include <kernel.h>
#include <tty.h>
#include "interrupt.h"
#include "serial.h"

static unsigned char tbuf1[TTYSIZ];

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
  outl(UART_BASE(minor - 1) + UART_DR, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
  /* For a CPU this fast we should switch to IRQ driven ? */
  if (inl(UART_BASE(minor - 1) + UART_FR) & UART_FR_TXFF)
    return TTY_READY_SOON;
  return TTY_READY_NOW;
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
  /* Kernel console I/O is blocking. */
  while(tty_writeready(1) != TTY_READY_NOW)
    tty_putc(1, c);
}

static irqreturn_t serial_isr(unsigned int irq, void *dev_id, uint32_t *regs)
{
  uint32_t mis;
  unsigned u;
  unsigned int dev = (unsigned int)dev_id;
  uint32_t base = UART_BASE(dev - 1);

  for (u = 0U; u < 256U; u++) {
    mis = inl(base + UART_MIS);
    outl(base + UART_ICR, mis);
    if (mis & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
      while (!(inl(base + UART_FR) & UART_FR_RXFE))
        /* Should do something with errors I guess one day */
        tty_inproc(dev, inl(base + UART_DR));
      continue;
    }
    if (mis & UART_MIS_TXMIS) {
      tty_outproc(dev);
      continue;
    }
    break;
  }
  return IRQ_HANDLED;
}

void serial_late_init(void)
{
  uint32_t u;
  uint32_t base = UART_BASE(0);

  outl(base + UART_IFLS, 
             UART_IFLS_TXIFLSEL_18TH | UART_IFLS_RXIFLSEL_18TH);
  outl(base + UART_IM, UART_IM_RXIM | UART_IM_RTIM);
  u = inl(base + UART_LCRH);
  u |= UART_LCRH_FEN;
  outl(base + UART_LCRH, u);
  u = inl(base + UART_CTL);
  u |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
  outl(base + UART_CTL, u);
  request_irq(IRQ_UART0, serial_isr, (void *)1);
}

#define SYSCON_RCGUART	0x400FE618
#define SYSCON_PCUART	0x400FE918
#define SYSCON_PRUART	0x400FEA18

void serial_early_init(void)
{
  uint32_t ctl;

  /* Power up the UART */
  outmod32(SYSCON_PCUART, 1, 1);
  outmod32(SYSCON_RCGUART, 1, 1);

  /* Put the console on the right pins and power them up */

  gpio_altfunc(GPIO_PORT('A'), 0, 1);
  gpio_altfunc(GPIO_PORT('A'), 1, 1);

  /* Can't use the UART until it finishes coming out of power gating */
  while (!(inl(SYSCON_PRUART) & 1));

  /* Disable the UART by clearing the UARTEN bit in the UART CTL register */
  ctl = inl(UART_BASE(0) + UART_CTL);
  ctl &= ~UART_CTL_UARTEN;
  outl(UART_BASE(0) + UART_CTL, ctl);

  /* We need 115200 baud, and 16 samples per bit (like a 16x50) and we run
     at 120MHz */
  outl(UART_BASE(0) + UART_IBRD, 65);
  outl(UART_BASE(0) + UART_FBRD, 192000);
  /* 8N1 FIFO on */
  outl(UART_BASE(0) + UART_LCRH, (3 << 5) | 0x10);

  ctl |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);
  outl(UART_BASE(0) + UART_CTL, ctl);
}
