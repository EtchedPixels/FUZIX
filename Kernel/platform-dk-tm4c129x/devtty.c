#include <stddef.h>
#include <stdint.h>

#include <kernel.h>
#include <tty.h>

#include "serial.h"

#include "config.h"

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
  serial_putc((uint32_t)(c));
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
  return serial_txnotfull() ? TTY_READY_NOW : TTY_READY_SOON;
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
    tty_putc(1U, '\r');
  tty_putc(1U, c);
}
