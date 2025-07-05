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

 #define _CH375_PRIVATE
 
 #include "ch375.h"
 
 
#ifdef CONFIG_CH375

uint8_t ch_rd = CH375_CMD_RD_USB_DATA;
uint8_t ch_wd = CH375_CMD_WR_USB_DATA7;

uint8_t ch375_rdata(uint8_t dev)
{
    uint16_t count = 0x8000;
    /*kprintf("ch375_rdata: %u\n", dev);*/
    if (ch375_comm_type[dev] == CH375_COMM_SERIAL)
    {
        while(--count && (in16(ch375_sports[dev]) == CH375_SERIAL_WAIT));
        if (count == 0) {
            kprintf("ch375_serial: timeout.\n");
            return 0xFF; 
        }
    }
    return in16(ch375_dports[dev]);
}

uint8_t ch375_rstatus(uint8_t dev)
{
    /*kprintf("ch375_rstatus: %u\n", dev);*/
    switch (ch375_comm_type[dev])
    {
    case CH375_COMM_PARALLEL:
        return in16(ch375_sports[dev]);
        break;
    case CH375_COMM_SERIAL:
        return ch375_rdata(dev);
        break;
    default:
        break;
    }
} 

void ch375_wdata(uint8_t dev, uint8_t data)
{
    out16(ch375_dports[dev], data);
    /*kprintf("ch375_wdata: %u, %2x\n", dev, data);*/
}

void ch375_wcmd(uint8_t dev, uint8_t cmd)
{
    /*kprintf("ch375_wcmd: %u, %2x\n", dev, cmd);*/
    switch (ch375_comm_type[dev])
    {
        case CH375_COMM_PARALLEL:
            out16(ch375_sports[dev], cmd);
        break;
        case CH375_COMM_SERIAL:
            ch375_wdata(dev, 0x57);
            ch375_wdata(dev, 0xAB);
            ch375_wdata(dev, cmd);
        break;
    }
}


 /* Some guesswork here on how the get status polling is meant to work */
 uint8_t ch375_rpoll(uint_fast8_t dev)
 {
     uint16_t count = 0x8000;
     uint8_t r;
     while(--count && ((((r = ch375_rstatus(dev)) & 0x80) != 0 ))) nap2();
     if (count == 0) {
         kprintf("ch375: timeout. rstat=%2x\n", r);
         return 0xFF;
     }
     /*kprintf("rstat=%2x\n", r);*/
     /* Get interrupt status, and clear interrupt */
     ch375_wcmd(dev, CH375_CMD_GET_STATUS);
     nap2();
     r = ch375_rdata(dev);
     /*kprintf("ch375_rpoll %u,%u,%2x\n", dev, count, r);*/
     return r;
 }

int ch375_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
 {
     uint_fast8_t n = 0;
     uint_fast8_t r;
    
    td_io_data_reg = ch375_dports[dev];
    td_io_data_count = 8;
    /*kprintf("dr:%x\n",ch375_data_reg);*/
    if (is_read)
         ch375_wcmd(dev, CH375_CMD_DISK_READ);
     else
         ch375_wcmd(dev, CH375_CMD_DISK_WRITE);
     nap2();
     ch375_wdata(dev, lba);
     ch375_wdata(dev, lba >> 8);
     ch375_wdata(dev, lba >> 16);
     ch375_wdata(dev, lba >> 24);
     ch375_wdata(dev, 1);
     for (n = 0; n < 8; n++) {
         r = ch375_rpoll(dev);
         if (is_read) {
            /*kprintf("rd %u\n", n);*/
             if (r != CH375_USB_INT_DISK_READ){
                /*kprintf("r != CH375_USB_INT_DISK_READ = %2x\n", r);*/
                continue;
             }
             ch375_wcmd(dev, ch_rd);
             nap2();
             r = ch375_rdata(dev);	/* Throw byte count away - always 64 */
             if (r != 64) {
                 kprintf("weird rd len %d\n", r);
                 continue;
             }
             /*kprintf("rblock(%x)\n", dptr);*/
             td_io_rblock(dptr);
             ch375_wcmd(dev, CH375_CMD_DISK_RD_GO);
         } else {
            /*kprintf("wr\n");*/
             if (r != CH375_USB_INT_DISK_WRITE)
                 continue;
             ch375_wcmd(dev, ch_wd);
             nap2();
             ch375_wdata(dev, 0x40);	/* Send write count */
             td_io_wblock(dptr);
             ch375_wcmd(dev, CH375_CMD_DISK_WR_GO);
         }
         dptr += 0x40;
     }
     r = ch375_rpoll(dev);
     if (r != CH375_USB_INT_SUCCESS) {
         kprintf("ch375: error %d\n", r);
         return 0;
     }
     return 1;        
 }
 
 
 #endif
