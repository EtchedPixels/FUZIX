/*
 *	CH375 and CH376 driver for Fuzix.
 *
 *	Controller in parallel mode and running internal block firmware rather
 *	than being a raw USB controller.
 *
 *	Caller provides
 *		ch375_rdata();
 *		ch375_rstatus();
 *		ch375_wcmd(cmd);
 *		ch375_wdata(data);
 *		nap20();	nap 20us or so 
 *
 *	Plus the usually asm block copies for the read/write of the block
 *
 *	TODO: handle hotplugging once we get the infrastructure
 *
 *	Q: do we need to check 0x10 status before write  or read ? Push it to host
 *	as 1,5us wont be a problem in many cases
 */

#include <kernel.h>
#include <tinydisk.h>
#include "ch375.h"

static uint8_t ch_ver;
static uint8_t ch_rd = 0x28;
static uint8_t ch_wd = 0x2B;
static uint8_t ch_dev = 0xFF;

/* Some guesswork here on how the get status polling is meant to work */
static uint8_t ch375_rpoll(void)
{
    uint16_t count = 0x8000;
    while(--count && ((ch375_rstatus() & 0x80) == 0x80));
    if (count == 0) {
        kprintf("ch375: timeout.\n");
        return 0xFF;
    }
    /* Get interrupt status, and clear interrupt */
    ch375_wcmd(0x22);
    return ch375_rdata();
}

static uint8_t ch375_cmd_r(uint8_t cmd)
{
    ch375_wcmd(cmd);
    return ch375_rpoll();
}

static void ch375_cmd2(uint8_t cmd, uint8_t data)
{
    ch375_wcmd(cmd);
    ch375_wdata(data);
}

static uint8_t ch375_cmd2_r(uint8_t cmd, uint8_t data)
{
    ch375_wcmd(cmd);
    ch375_wdata(data);
    return ch375_rpoll();
}

static int ch375_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
{
    uint_fast8_t n = 0;
    uint_fast8_t r;

    if (is_read)
        ch375_wcmd(0x54);
    else
        ch375_wcmd(0x56);
    ch375_wdata(lba);
    ch375_wdata(lba >> 8);
    ch375_wdata(lba >> 16);
    ch375_wdata(lba >> 24);
    ch375_wdata(1);
    if (ch375_rdata() != 0x1D)
        return 0;
    for (n = 0; n < 8; n++) {
        if (is_read) {
            ch375_rblock(dptr);
            r = ch375_cmd_r(0x55);
        } else {
            ch375_wblock(dptr);
            r = ch375_cmd_r(0x57);
        }
        dptr += 0x40;
    }
    if (r != 0x14) {
        kprintf("ch375: error %d\n", r);
        return 0;
    }
    return 1;        
}

uint_fast8_t ch375_probe(void)
{
    uint_fast8_t chip = 5;
    uint_fast8_t n;
    uint_fast8_t r;

    if (ch375_cmd2_r(0x06, 0x55) != 0xAA)
        return 0;
    ch_ver = ch375_cmd_r(0x01);
    if (ch_ver == 0xFF)
        return 0;
    /* Reset the bus */
    ch375_cmd2(0x15, 0x07);
    nap20();	/* 20 us */
    ch375_cmd2(0x15, 0x06);
    /* 376 - update commands to use */
    if ((ch_ver & 0xC0) == 0x40) {
        ch_rd = 0x27;
        ch_wd = 0x2C;
        chip = 6;
    }
    kprintf("ch37%d: firmware version %d\n", ch_ver & 0x3F);
    nap20();
    /* Can take a few goes */
    for (n = 0; n < 10; n++) {
        r = ch375_cmd_r(0x51);
        if (r != 0x14)
            return 0;
        nap20();
    }
    /* And done */
    ch_dev = td_register(ch375_xfer, 1);
    return 1;
}
