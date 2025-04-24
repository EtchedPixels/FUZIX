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

#define CH375_CMD_GET_IC_VER            0x01
#define CH375_CMD_RESET_ALL             0x05
#define CH375_CMD_CHECK_EXIST           0x06
#define CH375_CMD_SET_USB_MODE          0x15
#define CH375_CMD_GET_STATUS            0x22
#define CH376_CMD_RD_USB_DATA           0x27
#define CH375_CMD_RD_USB_DATA           0x28
#define CH375_CMD_WR_USB_DATA7          0x2B
#define CH376_CMD_WR_HOST_DATA          0x2C
#define CH375_CMD_DISK_INIT             0x51
#define CH375_CMD_DISK_READ             0x54
#define CH375_CMD_DISK_RD_GO            0x55
#define CH375_CMD_DISK_WRITE            0x56
#define CH375_CMD_DISK_WR_GO            0x57

#define CH375_USB_INT_SUCCESS           0x14
#define CH375_USB_INT_CONNECT           0x15
#define CH375_USB_INT_DISK_READ         0x1D
#define CH375_USB_INT_DISK_WRITE        0x1E

/* TODO: 2 us delay should be implemented by platform */
#define nap2() nap20()

#ifdef CONFIG_CH375

static uint8_t ch_ver;
static uint8_t ch_rd = CH375_CMD_RD_USB_DATA;
static uint8_t ch_wd = CH375_CMD_WR_USB_DATA7;
static uint8_t ch_dev = 0xFF;

/* Some guesswork here on how the get status polling is meant to work */
static uint8_t ch375_rpoll(void)
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

static uint8_t ch375_cmd_r(uint8_t cmd)
{
    ch375_wcmd(cmd);
/*    kprintf("cmd_r %2x\n", cmd); */
    return ch375_rpoll();
}

static void ch375_cmd2(uint8_t cmd, uint8_t data)
{
/*    kprintf("cmd2: %2x %2x\n", cmd, data); */
    ch375_wcmd(cmd);
    nap2();
    ch375_wdata(data);
}

static uint8_t ch375_cmd2_r(uint8_t cmd, uint8_t data)
{
/*    kprintf("cmd2_r: %2x %2x\n", cmd, data); */
    ch375_wcmd(cmd);
    nap2();
    ch375_wdata(data);
    return ch375_rpoll();
}

static int ch375_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
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
