#include <kernel.h>
#include <nascom.h>

/*
 *	We can't really use it as a full RTC as it doesn't have a years
 *	field.
 */
uint_fast8_t plt_rtc_secs(void)
{
    register uint_fast8_t r, r2;
    
    do {
        r = in(clk_secs);
        r2 = in(clk_tsecs);
    } while (in(clk_secs) != r);
    return r + 10 * r2;
}

