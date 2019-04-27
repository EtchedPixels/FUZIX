#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

uaddr_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;
uint8_t ctc_present;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
  tty_poll(); 
  irqrestore(irq);
}

static int16_t timerct;

/* Call timer_interrupt at 10Hz */
static void timer_tick(uint8_t n)
{
	timerct += n;
	while (timerct >= 20) {
		timer_interrupt();
		timerct -= 20;
	}
}

void platform_interrupt(void)
{
	tty_poll();
	if (ctc_present) {
		uint_fast8_t n = 255 - ctc_check();
		timer_tick(n);
	}
}

