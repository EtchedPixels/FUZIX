#include <rp5c01.h>

__sfr __at (0xb4) RTC_REGSEL;
__sfr __at (0xb5) RTC_DATA;

uint8_t rp5c01_read_reg(uint8_t reg)
{
    RTC_REGSEL = reg & 0xf;
    return RTC_DATA;
}

void rp5c01_write_reg(uint8_t reg, uint8_t value)
{
    RTC_REGSEL = reg & 0xf;
    RTC_DATA = value;
}

uint_fast8_t plt_rtc_secs(void)
{
    return rp5c01_rtc_secs();
}
