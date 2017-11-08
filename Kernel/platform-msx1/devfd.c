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
    int ct = 0;
    int map = 0;

    is_read;
    
    if(rawflag) {
        if (d_blkoff(9))
            return -EIO;
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

