#include <kernel.h>

#define secs	0x22
#define tensecs	0x23
#define irq	0x2F

uint_fast8_t plt_rtc_secs(void)
{
    register uint_fast8_t r, r2;
    
    do {
        r = in(secs);
        r2 = in(tensecs);
    } while (in(secs) != r);
    return r + 10 * r2;
}
