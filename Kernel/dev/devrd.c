/*
 * memory backed block device driver
 *
 *     /dev/rd0      (block device)      RAM disk
 *     /dev/rd1      (block device)      ROM disk
 *
 * 2017-01-03 William R Sowerbutts, based on RAM disk code by Sergey Kiselev
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#define DEVRD_PRIVATE
#include "devrd.h"

static const uint32_t dev_limit[NUM_DEV_RD] = {
    DEV_RD_ROM_START+DEV_RD_ROM_SIZE, /* /dev/rd0: ROM */
    DEV_RD_RAM_START+DEV_RD_RAM_SIZE, /* /dev/rd1: RAM */
};

static const uint32_t dev_start[NUM_DEV_RD] = {
    DEV_RD_ROM_START, /* /dev/rd0: ROM */
    DEV_RD_RAM_START, /* /dev/rd1: RAM */
};

/* implements both rd_read and rd_write */
int rd_transfer(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    used(flag);

    /* check device exists; do not allow writes to ROM */
    if (minor == RD_MINOR_ROM && rd_reverse) {
        udata.u_error = EROFS;
        return -1;
    } else {
        rd_src_address = dev_start[minor];

        if (rawflag) {
            if (d_blkoff(9))
                return -1;
            /* rawflag == 1, userspace transfer */
        }
        rd_dst_userspace = rawflag;

        rd_dst_address = (uint16_t)udata.u_dptr;
        rd_src_address += ((uint32_t)udata.u_block) << BLKSHIFT;

        if (rd_src_address >= dev_limit[minor]) {
           udata.u_error = EIO;
           return -1;
        }
    }
    rd_platform_copy();
    return rd_cpy_count;
}

int rd_open(uint_fast8_t minor, uint16_t flags)
{
    flags; /* unused */

    switch(minor){
#if DEV_RD_RAM_PAGES > 0
        case RD_MINOR_RAM:
#endif
#if DEV_RD_ROM_PAGES > 0
        case RD_MINOR_ROM:
#endif
#if (DEV_RD_ROM_PAGES+DEV_RD_RAM_PAGES) > 0
            return 0;
#endif
        default:
            udata.u_error = ENXIO;
            return -1;
    }
}
