#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev;
uint8_t clk_irq;		/* Set if the clock can cause interrupts */
uint8_t plt_tick_present;

struct blkbuf *bufpool_end = bufpool + NBUFS;

/* FIXME: missing prototype ? */
extern int strcmp(const char *, const char *);
/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
	irqflags_t irq = di();
	tty_poll();
	sync_clock();
	irqrestore(irq);
}

void do_beep(void)
{
}

__sfr __at 0x21	clk_tenths;
__sfr __at 0x22 clk_secs;
__sfr __at 0x23 clk_tsecs;
__sfr __at 0x2F clk_stat;

uint8_t plt_param(char *p)
{
	if (strcmp(p, "clkint") == 0) {
		clk_irq = 1;
		/* FIXME: do a reset cycle first */
		clk_stat = 0x9;	/* Repeating 0.5 sec - really 0.516.6 sec */
		clk_stat | clk_stat | clk_stat;	/* Enable */
		plt_tick_present = 1;
		return 1;
	}
	used(p);
	return 0;
}

void plt_interrupt(void)
{
	/* We don't have interrupts for the keyboard */
	kbd_poll();
	tty_poll();
	if (clk_irq) {
		if (clk_stat & 0x08) {	/* Check 4 or 8 - need datasheet */
			/* Not ideal but we need to work out how to handle
			   the different clocks gracefully */
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
			/* Do we need to read again ? */
		}
	}
}

/*
 *	FIXME: reclaim to end of usable memory
 */
void plt_discard(void)
{
	bufptr bp = bufpool_end;
	extern uint16_t discard_size;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers reclaimed from discard\n", discard_size);

	bufpool_end += discard_size;	/* Reclaim the discard space */

	memset(bp, 0, discard_size * sizeof(struct blkbuf));
	/* discard_size is in discard so it dies here */
	for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}
