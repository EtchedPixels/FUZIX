/* 
 * Dummy hd driver code
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>

static int hd_transfer(bool is_read, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;
    uint16_t dptr;
    int dlen;
    int ct = 0;
    int map;
    
    is_read;

    if(rawflag) {
        if (d_blkoff(9))
            return -1;
        map = 1;
    }
    while (ct < block_xfer) {
        /* FIXME: Do stuff */
        block++;
        ct++;
    }
    return ct;
}

int hd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return hd_transfer(true, rawflag);
}

int hd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return hd_transfer(false, rawflag);
}

