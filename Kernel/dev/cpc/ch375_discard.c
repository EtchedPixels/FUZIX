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
#include <printf.h>
#include "ch375.h"

#ifdef CONFIG_CH375

static uint8_t ch_ver;
static uint8_t ch_rd = CH375_CMD_RD_USB_DATA;
static uint8_t ch_wd = CH375_CMD_WR_USB_DATA7;
static uint8_t ch_dev = 0xFF;


static void ch375_cmd2(uint8_t cmd, uint8_t data)
{
/*    kprintf("cmd2: %2x %2x\n", cmd, data); */
    ch375_wcmd(cmd);
    nap2();
    ch375_wdata(data);
}

uint_fast8_t ch375_probe(void)
{
    uint16_t i;
    uint_fast8_t chip = 5;
    uint_fast8_t r;

    /* Reset module in case of a crash. Takes 40 ms. */
    ch375_wcmd(CH375_CMD_RESET_ALL);
    for (i = 0; i < 2000; i++)
        nap20();

    ch375_cmd2(CH375_CMD_CHECK_EXIST, 0x55);
    nap2();
    r = ch375_rdata();
    if (r != 0xAA) {
/*        kprintf("ch375: response %2x not AA\n", r); */
        return 0;
    }

    ch375_wcmd(CH375_CMD_GET_IC_VER);	/* Version */
    nap2();
    ch_ver = ch375_rdata();
    kprintf("ch375: version %2x\n", ch_ver);
    if (ch_ver == 0xFF)
        return 0;
    /* 376 - update commands to use */
    if ((ch_ver & 0xC0) == 0x40) {
        ch_rd = CH376_CMD_RD_USB_DATA;
        ch_wd = CH376_CMD_WR_HOST_DATA;
        chip = 6;
    }
    kprintf("ch37%d: firmware version %d\n", chip, ch_ver & 0x3F);

    /* Enable USB host mode, reset USB bus */
    ch375_cmd2(CH375_CMD_SET_USB_MODE, 0x07);
    nap20();	/* 20 us */
    /* Enable USB host mode, automatically generating SOF */
    ch375_cmd2(CH375_CMD_SET_USB_MODE, 0x06);
    nap20();
    /* After setting USB mode 0x06 an interrupt is generated when
     * an USB storage device is present */
    r = ch375_rpoll();
    if (r != CH375_USB_INT_CONNECT) {
        return 0;
    }

    /* Initialize USB storage device */
    ch375_wcmd(CH375_CMD_DISK_INIT);
    r = ch375_rpoll();
    if (r != CH375_USB_INT_SUCCESS)
        return 0;

    /* And done */
    ch_dev = td_register(0, ch375_xfer, td_ioctl_none, 1);
    return 1;
}

#endif
