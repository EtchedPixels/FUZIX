#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev;
uint8_t clk_irq;		/* Set if the clock can cause interrupts */

struct blkbuf *bufpool_end = bufpool + NBUFS;

/* FIXME: missing prototype ? */
extern int strcmp(const char *, const char *);
/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
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

uint8_t platform_param(char *p)
{
	if (strcmp(p, "clkint") == 0) {
		clk_irq = 1;
		/* FIXME: do a reset cycle first */
		clk_stat = 0x9;	/* Repeating 0.5 sec - really 0.516.6 sec
				   so we need to fix clock tracking on
				   tickless FIXME */
		clk_stat | clk_stat | clk_stat;
		return 1;
	}
	used(p);
	return 0;
}

void platform_interrupt(void)
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
void platform_discard(void)
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

/*
 *	Logic for tickless system. If you have an RTC with a timer tick
 *	you can ignore this.
 */

static uint16_t newticks = 0xFFFF;
static uint16_t oldticks;

static uint8_t re_enter;

/*
 *	We have a 1/10ths timer which is nice but it's easy to overrun so we
 *	collect up tens/units/tenths.
 *
 *	Any ready can return 0xF not a BCD digit which means 'the time changed'
 */
static void sync_clock_read(void)
{
	uint8_t r[3];
	oldticks = newticks;

	do {
		r[0] = clk_tenths & 0x0F ;
		r[1] = clk_secs & 0x0F;
		r[2] = clk_tsecs & 0x0F;
	} while (r[0] == 0xF || r[1] == 0xF || r[2] == 0xF);

	newticks = r[0] + 10 * r[1] + 100 * r[2];
}

/*
 *	The OS core will invoke this routine when idle (via platform_idle) but
 *	also after a system call and in certain other spots to ensure the clock
 *	is roughly valid. It may be called from interrupts, without interrupts
 *	or even recursively so it must protect itself using the framework
 *	below.
 *
 *	Having worked out how much time has passed in 1/10ths of a second it
 *	performs that may timer_interrupt events in order to advance the clock.
 *	The core kernel logic ensures that we won't do anything silly from a
 *	jump forward of many seconds.
 *
 *	We also choose to poll the ttys here so the user has some chance of
 *	getting control back on a messed up process.
 */
void sync_clock(void)
{
	irqflags_t irq;
	int16_t tmp;

	if (clk_irq)
		return;

	irq = di();

	if (!re_enter++) {
		sync_clock_read();
		if (oldticks != 0xFF) {
			tmp = newticks - oldticks;
			if (tmp < 0)
				tmp += 600;
			while(tmp--) {
				timer_interrupt();
			}
			/* Poll the keyboard */
			platform_interrupt();
		}
	}
	re_enter--;
	irqrestore(irq);
}

/*
 *	This method is called if the kernel has changed the system clock. We
 *	don't work out how much work we need to do by using it as a reference
 *	so we don't care.
 */
void update_sync_clock(void)
{
}
