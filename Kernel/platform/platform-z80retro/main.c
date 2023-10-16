#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <z80retro.h>

uint16_t swap_dev = 0xFFFF;

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

void plt_discard(void)
{
	uint16_t space = 0xC000 - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;
#ifdef CONFIG_KMOD
	kmod_init(bufpool_end, (void *)0xC000);
#endif

	space /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", space);

	bufpool_end += space;

	memset( bp, 0, space * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

void plt_idle(void)
{
	__asm halt __endasm;
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

void plt_interrupt(void)
{
	static uint8_t n;
	tty_drain_sio();
	n++;
	if (n == 24) {
		timer_interrupt();
		n = 0;
	}
}
