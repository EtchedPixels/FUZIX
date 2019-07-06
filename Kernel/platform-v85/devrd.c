/*
 *	RAM drive (512K)
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devrd.h>

static uint_fast8_t rd_sizeh;

static int rd_transfer(uint8_t minor, bool is_read, uint_fast8_t rawflag)
{
    uint_fast8_t page = 0;
    uint16_t ct = 0;
    if (rawflag == 2)
        page = swappage;
    else if (rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
        page = udata.u_page;
    }
    /* Work in 256 byte pages for ease. Max size is 8MB so can't overflow */
    udata.u_nblock <<= 1;
    udata.u_block <<= 1;
    while(ct < udata.u_nblock) {
        (is_read?rd_input:rd_output)(udata.u_dptr, udata.u_block, page);
        udata.u_dptr += 256;
        udata.u_block++;
        ct++;
    }
    return ct << 8;
}

int devrd_open(uint8_t minor, uint16_t flag)
{
    if (minor || !rd_sizeh) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int devrd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return rd_transfer(minor, true, rawflag);
}

int devrd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;rawflag;minor;
    return rd_transfer(minor, false, rawflag);
}

void devrd_init(void)
{
    uint_fast8_t i;
    for (i = 0; i < 8; i++) {
        if (rd_present(i) == 0)
            break;
        rd_sizeh += 8;	/* 8 * 64K per board */
    }
    if (rd_sizeh)
        kprintf("rd0: %dKb available.\n", rd_sizeh * 64);
}
