/*
 * rp5c01a-rtc
 * Real Time Clock with internal non-volatile RAM
 */
#include <rp5c01.h>


uint8_t rp5c01_rtc_secs(void)
{
    uint8_t sl, rv;

    /* set mode 00 */
    rp5c01_write_reg(MODE_SEL, TIMER_ENABLE | MODE00);

    /* BCD encoded */
    do {
        sl = rp5c01_read_reg(REG_1_SEC_CTR);
        rv = sl + rp5c01_read_reg(REG_10_SEC_CTR) * 10;
    } while (sl != rp5c01_read_reg(REG_1_SEC_CTR));

    /* rp5c01_write_reg(MODE_SEL, TIMER_ENABLE | MODE01); */

    return rv;
}

#ifdef CONFIG_RTC_RP5C01_NVRAM
void rp5c01_nvram_write(uint8_t addr, uint8_t value)
{
    if (addr > 0xC)
	return;

    /* store 4 msb in mode 10 regs, 4 lsb in mode 01 regs */
    rp5c01_write_reg(MODE_SEL, TIMER_ENABLE | MODE10);
    rp5c01_write_reg(addr, value & 0xF0 >> 4);
    rp5c01_write_reg(MODE_SEL, TIMER_ENABLE | MODE11);
    rp5c01_write_reg(addr, value & 0x0F);
}

uint8_t rp5c01_nvram_read(uint8_t addr)
{
    uint8_t val, data;

    if (addr > 0xC)
	return;

    /* read 4 msb from mode 10 regs, 4 lsb from mode 01 regs */
    rp5c01_write_reg(MODE_SEL, TIMER_ENABLE | MODE10);
    val = rp5c01_read_reg(addr);
    data = val << 4;

    rp5c01_write_reg(MODE_SEL, TIMER_ENABLE | MODE11);
    val = rp5c01_read_reg(addr);
    data |= val & 0x0F;

    return data;
}
#endif

