#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <tty.h>
#include <devtty.h>

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xE000;
uint8_t need_resched = 0;

uint8_t fdc765_present;

void plt_discard(void)
{
	while (bufpool_end < (struct blkbuf *) (KERNTOP - sizeof(struct blkbuf))) {
		memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
		bufpool_end->bf_busy = BF_FREE;	/* redundant when BF_FREE == 0 */
#endif
		bufpool_end->bf_dev = NO_DEVICE;
		bufpool_end++;
	}
}

void plt_idle(void)
{
	irqflags_t irq = di();
	handle_keys();
	irqrestore(irq);
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

/* Tidy up once it's working and we've got our ints set up the way we need */
__sfr __banked __at 0xD019 intack;
__sfr __banked __at 0xDC0D intcia;

void plt_interrupt(void)
{
	//handle_keys();
	//timer_interrupt();
	intack = 0xFF;
	intcia;
}

void do_beep(void)
{
}

uint8_t petscii(uint8_t x)
{
	x &= 127;
	if (x > 96)
		x -= 96;
	return x;
}