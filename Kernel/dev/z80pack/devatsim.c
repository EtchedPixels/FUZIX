#include <kernel.h>
#include <kdata.h>
#include <netdev.h>
#include <net_at.h>

/*
 *	Implement a very simple interface to serial 3 on Z80pack, with a
 *	small patch that sets bit 2 to indicate channel connected.
 *
 *	This is used to debug the ATD ipaddr interface.
 *
 *	Note: the interface *must* be hardware flow controlled and 8bit clean
 */

__sfr __at 44 status;
__sfr __at 45 data;

static uint8_t up;
static uint8_t poll;

void netat_nowake(void)
{
  poll = 0;
}

void netat_wake(void)
{
  poll = 1;
}

uint8_t netat_byte(void)
{
  return data;
}

uint8_t netat_ready(void)
{
  return (status & 0x01) ? 1 : 0;
}

static void netat_drop(void)
{
    if (up) {
      up = 0;
      netat_hangup();
    }
}

void netat_flush(void)
{
  while((status & 5) == 5)
    data;
}

void netat_poll(void)
{
  uint8_t st = status;
  if (!(st & 4)) {
    netat_drop();
    return;
  }
  up = 1;
  while (poll && (status & 1))
      netat_event();
}

void netat_outbyte(uint8_t c)
{
  uint8_t st;
  do {
    st = status;
    if (!(st & 4)) {
      netat_drop();
      return;
    }
  } while (!(st & 2));
  data = c;
}
