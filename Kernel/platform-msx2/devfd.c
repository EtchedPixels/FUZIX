/* 
 * Dummy fd driver code
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

static int fd_transfer(bool is_read, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;
    uint16_t dptr;
    int dlen;
    int ct = 0;
    int map;

    is_read;
    
    /* FIXME: raw is broken unless nicely aligned */
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
        /* FIXME: Do stuff */
        block++;
        ct++;
    }
    return ct;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return fd_transfer(true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return fd_transfer(false, rawflag);
}

