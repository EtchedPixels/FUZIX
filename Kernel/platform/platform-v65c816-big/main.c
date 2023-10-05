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
 * User in banks 3-125 or thereabouts
 * 2 is kdata
 * 1 is kcode
 * 0 is all the stacks and DP stuff we can't put elsehwere
 */

void pagemap_init(void)
{
    int i;
    /* Bank 0 is the kernel */
    for (i = MAX_MAPS ; i > 2; i--)
        pagemap_add(i);
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}
