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
    int dlen;
    int ct = 0;
    int map = 0;
    
    is_read;

    /* FIXME: raw is broken unless nicely aligned */
    if (rawflag) {
        if (d_blkoff(9))
            return -1;
        map = 1;
    }

    block = udata.u_block;        
    while (ct < udata.u_nblock) {
        /* FIXME: Do stuff */
        block++;
        ct++;
    }
    return ct << BLKSHIFT;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return hd_transfer(true, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return hd_transfer(false, rawflag);
}

