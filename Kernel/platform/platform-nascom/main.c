#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinysd.h>
#include <z80softspi.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev = 0xFFFF;
uint8_t clk_irq;		/* Set if the clock can cause interrupts */
uint8_t plt_tick_present;

struct blkbuf *bufpool_end = bufpool + NBUFS;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
	irqflags_t irq = di();
	tty_poll();
	kbd_poll();
	sync_clock();
	irqrestore(irq);
}

void do_beep(void)
{
}

#define clk_tenths	0x21
#define clk_secs	0x22
#define clk_tsecs	0x23
#define clk_stat	0x2F

int strcmp(const char *s1, const char *s2)
{
  char c1, c2;

  while((c1 = *s1++) == (c2 = *s2++) && c1);
  return c1 - c2;
}

uint_fast8_t plt_param(char *p)
{
	if (strcmp(p, "clkint") == 0) {
		out(clk_irq, 1);
		/* FIXME: do a reset cycle first */
		out(clk_stat, 0x9);	/* Repeating 0.5 sec - really 0.516.6 sec */
		in(clk_stat);
		in(clk_stat);
		in(clk_stat);	/* Enable */
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
			timer_interrupt();
			/* Do we need to read again ? */
		}
	}
}

/*
 *	Reclaim to end of usable memory. We currently require RAM is present
 *	to F000 anyway.
 */
void plt_discard(void)
{
	register bufptr bp = bufpool_end;

	while(bp + 1 < (void *)0xF000) {
		memset(bp, 0, sizeof(struct blkbuf));
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
		bp++;
	}
	kprintf("%d buffers reclaimed from discard\n", bp - bufpool_end);
	bufpool_end = bp;
}

