#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint16_t ramtop = PROGTOP;

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

/*
 * 7 banks, kernel in bank 0
 */

void pagemap_init(void)
{
    int i;
    /* Bank 0 is the kernel */
    for (i = MAX_MAPS ; i > 0; i--)
        pagemap_add(i);
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}
