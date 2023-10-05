#include <kernel.h>

__sfr __at 0x22 secs;
__sfr __at 0x23 tensecs;
__sfr __at 0x2F irq;

uint8_t plt_rtc_secs(void)
{
    uint8_t r, r2;
    
    do {
        r = secs;
        r2 = tensecs;
    } while (secs != r);
    return r + 10 * r2;
}
