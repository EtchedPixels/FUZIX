/*
 *	DS1302 bit ops
 */

#include <kernel.h>
#include <ds1302.h>

#define	ds1302	((*(volatile uint8_t *)0xFFFF8043))

static uint8_t ds1302_bits;

static void ds1302_set_bit(uint8_t mask, uint8_t onoff)
{
    if (onoff)
        ds1302_bits |= mask;
    else
        ds1302_bits &= ~mask;
    ds1302 = ds1302_bits;
}

void ds1302_set_clk(bool state)
{
    ds1302_set_bit(4, state);
}

void ds1302_set_ce(bool state)
{
    ds1302_set_bit(8, !state);
}

void ds1302_set_data(bool state)
{
    ds1302_set_bit(1, state);
}

void ds1302_set_driven(bool state)
{
    ds1302_set_bit(2, state);
}

bool ds1302_get_data(void)
{
    return ds1302 & 1;
}
