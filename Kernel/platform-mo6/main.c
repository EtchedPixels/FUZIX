#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

uint8_t membanks;
uint16_t swap_dev;

void plt_idle(void)
{
    irqflags_t irq = di();
    poll_keyboard();
    irqrestore(irq);
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

uint8_t vtattr_cap;