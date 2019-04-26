#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint8_t kernel_flag = 1;

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
    int i;
    /* 32-35 are the kernel, 36-38 / 39-41 / etc are user plus a bit of spare */
    for (i = 0; i < 9; i++)
        pagemap_add(36 + i * 3);
}

void map_init(void)
{
}

uint8_t platform_param(char *p)
{
    return 0;
}
