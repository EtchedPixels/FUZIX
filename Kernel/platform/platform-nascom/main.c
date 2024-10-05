#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinysd.h>
#include <nascom.h>
#include <fdc80.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev = 0xFFFF;
static uint8_t clk_nmi;		/* Set if the clock can cause an NMI */
uint8_t plt_tick_present;
uint8_t have_mm58174;

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


int strcmp(const char *s1, const char *s2)
{
  char c1, c2;

  while((c1 = *s1++) == (c2 = *s2++) && c1);
  return c1 - c2;
}

/* TODO: move to discard ? */
uint_fast8_t plt_param(char *p)
{
	if (strcmp(p, "rtcnmi") == 0) {
		plt_tick_present = 1;
		clk_nmi = 1;	/* Jumpered for NMI */
		plt_enable_nmi();
		kputs("NMI based RTC enabled.\n");
		return 1;
	}
	return 0;
}

/* These are used to avoid NMI in the tight inner floppy transfer loop */
void plt_disable_nmi(void)
{
	if (clk_nmi) {
		out(clk_stat, 0x00);
		in(clk_stat);
		in(clk_stat);
		in(clk_stat);
	}
}

void plt_enable_nmi(void)
{
	if (clk_nmi) {
		out(clk_stat, 0x09);
		in(clk_stat);
		in(clk_stat);
		in(clk_stat);
	}
}

void plt_interrupt(void)
{
	uint8_t c;
	/* We don't have interrupts for the keyboard */
	kbd_poll();
	tty_poll();
	if (clk_nmi && nmi) {
		c = nmi;
		nmi = 0;
		/* This isn't ideal at all */
		/* The 58174 runs at 0.5Hz */
		while(c--) {
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
		}
		/* Do we need to read again ? */
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

