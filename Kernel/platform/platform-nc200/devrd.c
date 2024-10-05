/* 
 * NC100 RD PCMCIA driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devrd.h>

/* Kernel page mapping */
static const uint8_t kmap[] = { 0x83, 0x84, 0x85 };

static int rd_transfer(bool is_read, uint_fast8_t rawflag)
{
    uint16_t dptr;
    int ct = 0;
    int map;
    const uint8_t *p = kmap;

    if(rawflag) {
        p = (const uint8_t *)&udata.u_page;
        if (d_blkoff(BLKSHIFT))
            return -1;
    }

    udata.u_block += 2*320;	/* ramdisc starts at 320K in */

    dptr = (uint16_t)udata.u_dptr;

    while (ct < udata.u_nblock) {
        uint16_t len;
        /* Pass the page to map for the data */
        map = p[(dptr >> 14)];
        len = 0x4000 - (dptr & 0x3FFF);
        if (len >= BLKSIZE)
            rd_memcpy(is_read, map, dptr, udata.u_block, BLKSIZE, 0, 0);
        else
            rd_memcpy(is_read, map, dptr, udata.u_block, len,
                p[(dptr >> 14) + 1], BLKSIZE - len);
        udata.u_block++;
        ct++;
        dptr += BLKSIZE;
    }
    return ct << BLKSHIFT;
}

int rd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return rd_transfer(true, rawflag);
}

int rd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return rd_transfer(false, rawflag);
}

