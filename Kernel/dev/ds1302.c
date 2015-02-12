/*-----------------------------------------------------------------------*/
/* DS1202 and DS1302 Serial Real Time Clock driver                       */
/* 2014-12-30 Will Sowerbutts                                            */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <ds1302.h>

void ds1302_send_byte(uint8_t byte)
{
    uint8_t i;

#ifdef DS1302_DEBUG
    kprintf("ds1302: send byte 0x%x\n", byte);
#endif
    /* drive the data pin */
    ds1302_set_pin_data_driven(true);

    /* clock out one byte, LSB first */
    for(i=0; i<8; i++){
        ds1302_set_pin_clk(false);
        /* for data input to the chip the data must be valid on the rising edge of the clock */
        ds1302_set_pin_data(byte & 1);
        byte >>= 1;
        ds1302_set_pin_clk(true);
    }
}

uint8_t ds1302_receive_byte(void)
{
    uint8_t i, b;

    /* tri-state the data pin */
    ds1302_set_pin_data_driven(false);

    /* clock in one byte, LSB first */
    b = 0;
    for(i=0; i<8; i++){
        ds1302_set_pin_clk(false);
        b >>= 1;
        /* data output from the chip is presented on the falling edge of each clock */
        /* note that output pin goes high-impedance on the rising edge of each clock */
        if(ds1302_get_pin_data())
            b |= 0x80;
        ds1302_set_pin_clk(true);
    }

    return b;
}

uint8_t uint8_from_bcd(uint8_t value)
{
    return (value & 0x0F) + (10 * (value >> 4));
}

void ds1302_read_clock(uint8_t *buffer, uint8_t length)
{
    uint8_t i;
    irqflags_t irq = di();

    ds1302_set_pin_ce(true);
    ds1302_send_byte(0x81 | 0x3E); /* burst read all calendar data */
    for(i=0; i<length; i++){
        buffer[i] = ds1302_receive_byte();
#ifdef DS1302_DEBUG
        kprintf("ds1302: received byte 0x%x index %d\n", buffer[i], i);
#endif
    }
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
    irqrestore(irq);
}

/* define CONFIG_RTC in platform's config.h to hook this into timer.c */
uint8_t rtc_secs(void)
{
    uint8_t buffer;
    ds1302_read_clock(&buffer, 1);   /* read out only the seconds value */
    return uint8_from_bcd(buffer & 0x7F); /* mask off top bit (clock-halt) */
}
