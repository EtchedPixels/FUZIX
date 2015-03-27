#include <kernel.h>
#include <sio.h>

__sfr __at 0x00 baudrate;
__sfr __at 0x15 artmr;

/* Select SIO */
int select_sio(void)
{
  /* FIXME: save old modem bits, baud etc. We can't do this via hw
     so probably need some kind of tty callbacks here */
  baudrate = 0xB0;	/* 38400 */
  artmr = 0x04;		/* 8N1 */
  /* FIXME: disable ART IRQ */
  return 0;
}

void deselect_sio(int old)
{
  /* FIXME */
  used(old);
}

__sfr __at 0x14 artwr;
__sfr __at 0x15 artsr;

void sio_write(uint8_t *buf, int len)
{
  while(len--) {
    while(!(artsr & 1));
    artwr = *buf++;
  }
}

int sio_read(uint8_t *buf, int len)
{
  /* FIXME: timeouts etc */
  while(len--) {
    while(!(artsr & 2))
      *buf++ = artwr;
  }
  return 0;
}
