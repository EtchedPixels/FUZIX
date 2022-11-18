/*
 *	DS12885 glue for platforms which have MMIO
 */

#include "kernel.h"

#ifdef CONFIG_RTC_DS12885

#include "ds12885.h"

uint_fast8_t ds12885_read(uint_fast8_t port)
{
    *(volatile uint8_t *)RTC_ADDR = port;
    return *(volatile uint8_t *)RTC_DATA;
}

void ds12885_write(uint_fast8_t port, uint_fast8_t value)
{
    *(volatile uint8_t *)RTC_ADDR = port;
    *(volatile uint8_t *)RTC_DATA = value;
}

#endif