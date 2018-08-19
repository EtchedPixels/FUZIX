#include <kernel.h>
#include <kdata.h>
#include <devide.h>

__sfr __at 0xF5 ide_addr;
__sfr __at 0xF6 ide_high;
__sfr __at 0xF7 ide_low;

uint8_t devide_readb(uint8_t reg)
{
    volatile uint8_t dummy;
    ide_addr = reg;
    dummy = ide_high;
    return ide_low;
}

void devide_writeb(uint8_t reg, uint8_t value)
{
    ide_addr = reg;
    ide_high = 0;
    ide_low = value;
}
