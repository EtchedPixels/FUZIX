/* 
 * Memotech Silicon Disk Driver
 *
 * FIXME: add swap support to this driver for boxes with < 512K RAM.
 * FIXME: Check presence of device by probing ?
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devsil.h>

#define NUM_SIL		4

static int sil_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    int ct = 0;
    int map = 0;
    uint8_t unit = 0x50 + 4 * minor;

    if(rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
        map = udata.u_page;
    }
    if (rawflag == 2)
        return -1;

    while (ct < udata.u_nblock) {
        sil_memcpy(is_read, map, (uint16_t)udata.u_dptr, udata.u_block, unit);
        udata.u_dptr += BLKSIZE;
        udata.u_block++;
        ct++;
    }
    return ct << BLKSHIFT;
}

int sil_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor >= NUM_SIL) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int sil_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return sil_transfer(minor, true, rawflag);
}

int sil_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return sil_transfer(minor, false, rawflag);
}
