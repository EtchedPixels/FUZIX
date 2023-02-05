#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

uint8_t system_id;
uint8_t lower;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}

void do_beep(void)
{
}

void plt_discard(void)
{
}
