#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint8_t kernel_flag = 0;

void platform_idle(void)
{
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
    /* 0-3 kernel, 4-9 Bank 0, 10-15 bank 1 */
    pagemap_add(4);
    pagemap_add(10);
}

void map_init(void)
{
}
