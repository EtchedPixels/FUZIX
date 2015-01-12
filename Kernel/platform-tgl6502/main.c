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
    /* We treat RAM as 2 48K banks + 16K of kernel data */
    /* 2-8 proc1, 9-15 proc2, 1 kernel data, 0 spare */
    pagemap_add(9);
    pagemap_add(2);
}

void map_init(void)
{
}
