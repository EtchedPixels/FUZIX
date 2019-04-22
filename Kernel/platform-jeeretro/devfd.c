/* 
 * Jeeretro fd driver
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>

static int hd_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    //uint8_t st;
    uint8_t map = 0;

    if(rawflag == 1) {
        if (d_blkoff(9))
            return -1;
        map = udata.u_page;
#ifdef SWAPDEV
    } else if (rawflag == 2) {		/* Swap device special */
        map = swappage;		        /* Acting on this page */
#endif
    }

    struct {
        uint8_t bank;
        uint8_t drive;
        uint8_t cmd;
        uint16_t dma;
        uint16_t sector;
    } __packed hdp;

    hdp.bank = map;
    hdp.drive = minor;
    hdp.dma = (uint16_t)udata.u_dptr;
    hdp.sector = udata.u_block;

    hdp.cmd = udata.u_nblock;
    if (!is_read)
        hdp.cmd |= 0x80;

#if 0
    kprintf("  hd%d: %s @ %d\t#%d addr %d:%x desc 0x%x raw %d\n",
            hdp.drive, is_read ? "r" : "W",
            hdp.sector, hdp.cmd & 0x7F, hdp.bank, hdp.dma, &hdp, rawflag);
#endif
    fd_cmd(&hdp);

    //st = 0; // XXX
    //if (st) {
    //    kprintf("hd%d: block %d, error %d\n", minor, udata.u_block, st);
    //    //return 0;
    //}

    udata.u_block += udata.u_nblock;
    return udata.u_nblock << 9;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    used(flag);
    if(minor >= 2) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return hd_transfer(true, minor, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return hd_transfer(false, minor, rawflag);
}
