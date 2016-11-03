#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint8_t kernel_flag = 0;

void platform_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    /* Really 8K banks 0,1,2,... but we pair them up so we have */
    /* 8 x 16 banks numbered 0,2,4,.... , 0 is the kernel, init starts in 2 */
    pagemap_add(14);
    pagemap_add(12);
    pagemap_add(10);
    pagemap_add(8);
    pagemap_add(6);
    pagemap_add(4);
    pagemap_add(2);
}

void map_init(void)
{
    udata.u_page = 0x0202;
    udata.u_page2 = 0x0202;
}

uint8_t platform_param(unsigned char *p)
{
    return 0;
}
