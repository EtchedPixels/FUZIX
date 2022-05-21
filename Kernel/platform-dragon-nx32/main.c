#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <carts.h>

uint8_t membanks;
uint8_t internal32k;
uint8_t system_id;
uint8_t cartslots = 1;
uint8_t carttype[4];
uint16_t cartaddr[4];
uint8_t bootslot = 0;
struct blkbuf *bufpool_end = bufpool + NBUFS;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
}

void do_beep(void)
{
}

/*
 *	Once we are about to load init we can throw the boot code away
 *	and convert it into disk cache. This gets us 7 or so buffers
 *	back which more than doubles our cache size !
 */
void plt_discard(void)
{
  extern uint8_t discard_size;
  bufptr bp = bufpool_end;

  kprintf("%d buffers reclaimed from discard\n", discard_size);

  bufpool_end += discard_size;	/* Reclaim the discard space */

  memset(bp, 0, discard_size * sizeof(struct blkbuf));
  /* discard_size is in discard so it dies here */
  for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
    bp->bf_dev = NO_DEVICE;
    bp->bf_busy = BF_FREE;
  }
}
