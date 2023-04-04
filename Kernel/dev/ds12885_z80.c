/*
 *	DS12885 glue for Z80. We can do this in C and get the right code
 *	for banked and non-banked.
 */

#include "kernel.h"

#ifdef CONFIG_RTC_DS12885

#include "ds12885.h"

__sfr __banked __at RTC_ADDR addr;
__sfr __banked __at RTC_DATA data;

uint_fast8_t ds12885_read(uint_fast8_t port)
{
    addr = port;
    return data;
}

void ds12885_write(uint_fast8_t port, uint_fast8_t value)
{
    addr = port;
    data = value;
}

#endif