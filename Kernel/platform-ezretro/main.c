#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uaddr_t ramtop = PROGTOP;

void pagemap_init(void)
{
    int i;
    for (i = 1; i <= MAX_MAPS; i++)
        pagemap_add(i);
}

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
    /* We don't want an idle poll and IRQ driven tty poll at the same moment */
    irqflags_t irq = di();
    tty_pollirq(); 
    irqrestore(irq);
}

void plt_interrupt(void)
{
    tty_pollirq();
    timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}

