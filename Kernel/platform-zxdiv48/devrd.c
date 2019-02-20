/* 
 * DivIDE Plus RAM bank ramdisc driver
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <divrd.h>

static int rd_transfer(uint8_t is_read, uint8_t rawflag)
{
    int ct = 0;

    /* It's a disk but only for swapping (and rd_io isn't general purpose) */
    if(rawflag != 2) {
        udata.u_error = EIO;
        return -1;
    }

    rd_wr = is_read;
    rd_dptr = udata.u_dptr;

    while (ct < udata.u_nblock) {
        rd_page = (udata.u_block >> 5) + 4;	/* 0-3 are kernel */
        rd_addr = (udata.u_block & 31) << 9;
        rd_io();        
        udata.u_block++;
        rd_dptr += BLKSIZE;
        ct++;
    }
    return ct << BLKSHIFT;
}

int rd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return rd_transfer(true, rawflag);
}

int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return rd_transfer(false, rawflag);
}

