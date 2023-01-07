#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devfd.h>

uint16_t ramtop = PROGTOP;
uint16_t vdpport = 0x02 + 256 * 40;	/* port and width */
uint8_t vdptype;
uint8_t has_mtxplus;
uint8_t membanks;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
    __asm
    halt
    __endasm;
}

/* Our timer actually runs at 62.5 ticks/second (125 with MTXplus turbo) */

static uint8_t tct, oct;

void plt_interrupt(void)
{
  extern uint8_t irqvector;

  /* Longer term we should split off the tty buffering. The challenge is
     testing it */
  if (irqvector == 1) {
    tty_interrupt();
    return;
  }
  /* TODO: This belongs in the asm irq vector front end patched in by port */
  if (has_mtxplus) {
    oct++;
    oct&=1;		/* For 8MHz right now */
    if (oct)
      return;
  }
  /* 125 pulses. We drop 5 per cycle: 25 50 75 100 125 */
  tct++;
  if (tct == 125)
    tct = 0;
  else if (tct != 25 && tct != 50 && tct != 75 && tct != 100) {
    kbd_interrupt();
    fd_motor_timer();
    timer_interrupt();
  }
}

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	Blow away the boot time discard area and the font we uploaded to the
 *	VDP.
 */
void plt_discard(void)
{
	uint16_t discard_size = (uint16_t)udata - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

uint8_t plt_canswapon(uint16_t dev)
{
	dev >>= 8;
	/* Swap to hard disc or silicon disc only */
	if (dev == 0 || dev == 8)
		return 1;
	return 0;
}
