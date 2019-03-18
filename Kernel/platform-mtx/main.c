#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devfd.h>

uint16_t ramtop = PROGTOP;
uint16_t vdpport = 0x02 + 256 * 40;	/* port and width */
uint8_t membanks;
uint16_t swap_dev = 0xFFFF;

void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

void platform_interrupt(void)
{
  extern uint8_t irqvector;

  if (irqvector == 1) {
    tty_interrupt();
    return;
  }
  kbd_interrupt();
  fd_motor_timer();
  timer_interrupt();
}

void do_beep(void)
{
}

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	Blow away the boot time discard area and the font we uploaded to the
 *	VDP.
 */
void platform_discard(void)
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
