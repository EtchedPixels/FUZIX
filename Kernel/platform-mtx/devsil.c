/* 
 * Memotech Silicon Disk Driver
 *
 * FIXME: would be sensible to add swap support to this driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devsil.h>

#define NUM_SIL		4

static int sil_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;
    uint16_t dptr;
    int dlen;
    int ct = 0;
    int map;

    if(rawflag) {
        dlen = udata.u_count;
        dptr = (uint16_t)udata.u_base;
        if (((uint16_t)dptr | dlen) & BLKMASK) {
            udata.u_error = EIO;
            return -1;
        }
        block = udata.u_offset >> 9;
        block_xfer = dlen >> 9;
        map = udata.u_page;
    } else { /* rawflag == 0 */
        dlen = 512;
        dptr = (uint16_t)udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        block_xfer = 1;
        map = 0;
    }
        
    while (ct < block_xfer) {
        sil_memcpy(is_read, map, dptr, block, 0x50 + 4 * minor);
        block++;
        ct++;
    }
    return ct;
}

int sil_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= NUM_SIL) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int sil_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return sil_transfer(minor, true, rawflag);
}

int sil_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return sil_transfer(minor, false, rawflag);
}

