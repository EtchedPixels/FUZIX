/* 
 * ROMdisc hack for testing
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devrd.h>

extern uint16_t romd_roff;
extern uint8_t romd_rmap;
extern uint8_t romd_bank;
extern uint8_t romd_mapu;

extern void __fastcall__ rd_copyin(uint16_t addr);


static int rd_transfer(bool is_read, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;
    uint16_t dptr;
    int dlen;
    int ct = 0;
    int map;
    irqflags_t irq;

    if(rawflag) {
        dlen = udata.u_count;
        dptr = (uint16_t)udata.u_base;
        /* Must be block aligned but otherwise we are happy */
        if ((udata.u_offset|dlen) & 0x1FF) {
            udata.u_error = EIO;
            return -1;
        }
        block = udata.u_offset >> 9;
        block_xfer = dlen >> 9;
        romd_mapu = 1;
    } else { /* rawflag == 0 */
        dlen = 512;
        dptr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        block_xfer = 1;
        romd_mapu = 0;
    }

    while (ct < block_xfer) {
        /* Offset of block within an 8K bank (high byte) */
        romd_roff = (block << 9) & 0x1FFF ;
        /* 8K block we need to select */
        romd_rmap = 0x48 + (block >> 4);
        /* Map it over a page we are not copying into */
        if (dptr >= 0xC000) {
            romd_roff += 0xA000;
            romd_bank = 0;
        } else {
            romd_roff += 0xC000;
            romd_bank = 1;
        }
        irq = di();
//        kprintf("RD: map %d, roff %x bank %d dptr %x\n",
//            romd_rmap, romd_roff, romd_bank, dptr);
        if (is_read) {
            rd_copyin(dptr);
        }
        irqrestore(irq);
        block++;
        ct++;
        dptr+=512;
    }
    return ct;
}

int rd_open(uint8_t minor, uint16_t flag)
{
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return rd_transfer(true, rawflag);
}

int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return rd_transfer(false, rawflag);
}

