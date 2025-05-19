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
#include <printf.h>
#include "ch375.h"



#ifdef CONFIG_CH375

static uint8_t ch_rd = CH375_CMD_RD_USB_DATA;
static uint8_t ch_wd = CH375_CMD_WR_USB_DATA7;

/* Some guesswork here on how the get status polling is meant to work */
uint8_t ch375_rpoll(void)
{
    uint16_t count = 0x8000;
    uint8_t r;
    while(--count && ((ch375_rstatus() & 0x80) != 0)) nap2();
    if (count == 0) {
        kprintf("ch375: timeout.\n");
        return 0xFF;
    }
    /* Get interrupt status, and clear interrupt */
    ch375_wcmd(CH375_CMD_GET_STATUS);
    nap2();
    r = ch375_rdata();
/*    kprintf("ch375_rpoll %2x", r); */;
    return r;
}


int ch375_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
{
    uint_fast8_t n = 0;
    uint_fast8_t r;

    if (is_read)
        ch375_wcmd(CH375_CMD_DISK_READ);
    else
        ch375_wcmd(CH375_CMD_DISK_WRITE);
    nap2();
    ch375_wdata(lba);
    ch375_wdata(lba >> 8);
    ch375_wdata(lba >> 16);
    ch375_wdata(lba >> 24);
    ch375_wdata(1);
    for (n = 0; n < 8; n++) {
        r = ch375_rpoll();
        if (is_read) {
            if (r != CH375_USB_INT_DISK_READ)
                return 0;
            ch375_wcmd(ch_rd);
            nap2();
            r = ch375_rdata();	/* Throw byte count away - always 64 */
            if (r != 64) {
/*                kprintf("weird rd len %d\n", r); */
                return 0;
            }
            ch375_rblock(dptr);
            ch375_wcmd(CH375_CMD_DISK_RD_GO);
        } else {
            if (r != CH375_USB_INT_DISK_WRITE)
                return 0;
            ch375_wcmd(ch_wd);
            nap2();
            ch375_wdata(0x40);	/* Send write count */
            ch375_wblock(dptr);
            ch375_wcmd(CH375_CMD_DISK_WR_GO);
        }
        dptr += 0x40;
    }
    r = ch375_rpoll();
    if (r != CH375_USB_INT_SUCCESS) {
        kprintf("ch375: error %d\n", r);
        return 0;
    }
    return 1;        
}

#endif
