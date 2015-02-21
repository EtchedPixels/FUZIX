/* 
 *	Disk driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);
static int hd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(true, minor, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(false, minor, rawflag);
}

/* hd is only used for swapping */
int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return hd_transfer(true, minor, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return hd_transfer(false, minor, rawflag);
}

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    /* Serial floppy driver - to write */
    is_read;
    minor;
    rawflag;
    return -EIO;
}

/* Easier to use statics as is this is single threaded and talking to asm
   code in a critical path */

uint16_t ramd_off;
uint16_t ramd_size;
uint16_t ramd_uaddr;

/* Really only ever used for swap */
static int hd_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    minor;

    if(rawflag == 1) {
        ramd_size = udata.u_count;
        ramd_uaddr = (uint16_t)udata.u_base;
        ramd_off = udata.u_offset << 1;
        /* Should check this higher up ? */
        if (((uint16_t)ramd_size | ramd_off) & BLKMASK)
            return -EIO;
    } else if (rawflag == 2) {		/* Swap device special */
        ramd_size = swapcnt << 9;
        ramd_uaddr = (uint16_t)swapbase;
        ramd_off = swapblk << 1;
    } else { /* rawflag == 0 */
        ramd_uaddr = (uint16_t)udata.u_buf->bf_data;
        ramd_off = udata.u_buf->bf_blk << 8;
        ramd_size = 512;
    }
    /* Block copy to/from the RAM disc I/O */
    if (is_read)
        hd_xferin();
    else
        hd_xferout();
    return ramd_size;
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= 3) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}
