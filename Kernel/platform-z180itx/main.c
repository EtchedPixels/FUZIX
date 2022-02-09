#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ps2kbd.h>
#include "config.h"
#include <z180.h>
#include "z180itx.h"

uint16_t ramtop = PROGTOP;
extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint8_t ps2kbd_present;
uint8_t ps2mouse_present;

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */

void platform_discard(void)
{
	while (bufpool_end <
	       (struct blkbuf *) (KERNTOP - sizeof(struct blkbuf))) {
		memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
		bufpool_end->bf_busy = BF_FREE;	/* redundant when BF_FREE == 0 */
#endif
		bufpool_end->bf_dev = NO_DEVICE;
		bufpool_end++;
	}
}

/* ITX 82C55 */

__sfr __at 0x7A ppi_c;

void platform_idle(void)
{
	__asm halt __endasm;
}

void z180_timer_interrupt(void)
{
	unsigned char a;

	/* we have to read both of these registers in order to reset the timer */
	a = TIME_TMDR0L;
	a = TIME_TCR;
	timer_interrupt();
	if (ps2kbd_present)
		ps2kbd_poll();
}

void platform_interrupt(void)
{
	switch (irqvector) {
	case Z180_INT_TIMER0:
		z180_timer_interrupt();
#ifdef CONFIG_NET_W5X00		
		w5x00_poll();
#endif		
		return;
	case Z180_INT_ASCI0:
		tty_pollirq_asci0();
		return;
	case Z180_INT_ASCI1:
		tty_pollirq_asci1();
		return;
	default:
		return;
	}
}
