/*
 *	CH375 and CH376 driver for Fuzix.
 */

 #include <kernel.h>
 #include <tinydisk.h>
 #include <printf.h>
 #include "ch375.h"
 

#ifdef CONFIG_CH375


extern uint8_t ch_rd;
extern uint8_t ch_wd;


void ch375_cmd2(uint_fast8_t dev, uint8_t cmd, uint8_t data)
 {
     /*kprintf("cmd2: %2x %2x\n", cmd, data);*/
     ch375_wcmd(dev, cmd);
     nap2();
     ch375_wdata(dev, data);
 }


 uint_fast8_t ch375_probe(void)
 {
     uint16_t i;
     uint_fast8_t chip = 5;
     uint_fast8_t r;
     uint8_t ch_ver;
     for (uint_fast8_t dev = 0; dev < CH375_DEVICES; dev++){

        kprintf("Probing ch375/ch376 device #%u\n", dev); 
        ch375_cmd2(dev, CH375_CMD_CHECK_EXIST, 0x55);
        nap2();
        r = ch375_rdata(dev);
        if (r != 0xAA) {
            kprintf("ch375: response %2x not AA\n", r);
            continue;
        }
        ch375_wcmd(dev, CH375_CMD_GET_IC_VER);	/* Version */
        nap2();
        ch_ver = ch375_rdata(dev);
        kprintf("ch375: version %2x\n", ch_ver);
        if (ch_ver == 0xFF)
            continue;
        /* 376 - update commands to use */
        if ((ch_ver & 0xC0) == 0x40) {
            ch_rd = CH376_CMD_RD_USB_DATA;
            ch_wd = CH376_CMD_WR_HOST_DATA;
            chip = 6;
        }
        kprintf("ch37%d: firmware version %d\n", chip, ch_ver & 0x3F);
        /* Enable USB host mode, reset USB bus */
        ch375_cmd2(dev, CH375_CMD_SET_USB_MODE, 0x07);
        nap20();	/* 20 us */
        /* Enable USB host mode, automatically generating SOF */
        ch375_cmd2(dev, CH375_CMD_SET_USB_MODE, 0x06);
        nap20();
        /* After setting USB mode 0x06 an interrupt is generated when
        * an USB storage device is present */
        r = ch375_rpoll(dev);
        if (r == 0x51)
            r = ch375_rpoll(dev);
        if (r != CH375_USB_INT_CONNECT) {
            continue;
        }
    
        /* Initialize USB storage device */
        ch375_wcmd(dev, CH375_CMD_DISK_INIT);
        r = ch375_rpoll(dev);
        if (r != CH375_USB_INT_SUCCESS)
            continue;
    
        /* And done */
        td_register(dev, ch375_xfer, td_ioctl_none, 1);
    }
    return 1;
 }
 
 #endif
