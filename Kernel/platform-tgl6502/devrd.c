/* 
 * ROMdisc hack for testing
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devrd.h>

static int rd_transfer(bool is_read, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;
    uint16_t dptr;
    int dlen;
    int ct = 0;
    int map;
    irqflags_t irq;
    uint8_t old;
    uint16_t romd_roff;
    uint8_t romd_rmap;

    /* RAW won't work yet this is just an initial hack */
    if(rawflag) {
        dlen = udata.u_count;
        dptr = (uint16_t)udata.u_base;
        if (dptr & 0x1FF) {
            udata.u_error = EIO;
            return -1;
        }
        block = udata.u_offset >> 9;
        block_xfer = dlen >> 9;
        map = 1;
    } else { /* rawflag == 0 */
        dlen = 512;
        dptr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        block_xfer = 1;
        map = 0;
    }

    while (ct < block_xfer) {
        /* Offset of block within an 8K bank (high byte) */
        romd_roff = (block << 9);
        /* 8K block we need to select */
        romd_rmap = 0x48 + (block >> 4);
        /* Hack for now for testing */
        irq = di();
        old = *(uint8_t *)0xFF91;
        *(uint8_t *)0xFF91 = romd_rmap;
        if (is_read)
            memcpy((void *)dptr, (void *)(0xE000 + romd_roff), 512);
        *(uint8_t *)0xFF91 = old;
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

