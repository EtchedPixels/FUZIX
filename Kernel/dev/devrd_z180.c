/* Z180 (Mark IV SBC & P112) memory driver
 *
 *     /dev/rd0      (block device)      RAM disk
 *     /dev/rd1      (block device)      ROM disk 
 *     /dev/physmem  (character device)  physical memory driver 
 *
 * 2017-01-05 William R Sowerbutts, based on Zeta-v2 RAM disk code by Sergey Kiselev
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#define DEVRD_PRIVATE
#include "devrd_z180.h"

/* if neither ROM nor RAM disks are enabled we can save a bit of code size */
#if (DEV_RD_RAM_PAGES+DEV_RD_ROM_PAGES) > 0
#define RD_BLOCK_DEVICES
#endif

#ifdef RD_BLOCK_DEVICES
static const uint32_t dev_limit[NUM_DEV_RD] = {
    (uint32_t)(DEV_RD_RAM_PAGES + DEV_RD_RAM_START) << 12, /* block /dev/rd0: RAM */
    (uint32_t)(DEV_RD_ROM_PAGES + DEV_RD_ROM_START) << 12, /* block /dev/rd1: ROM */
    1L << 20                                               /* char  /dev/physmem: full 1MB address space */
};

static const uint32_t dev_start[NUM_DEV_RD] = {
    (uint32_t)DEV_RD_RAM_START << 12,                      /* block /dev/rd0: RAM */
    (uint32_t)DEV_RD_ROM_START << 12,                      /* block /dev/rd1: ROM */
    0                                                      /* char  /dev/physmem: full 1MB address space */
};
#endif

int rd_transfer(uint8_t minor, uint8_t rawflag, uint8_t flag) /* implements both rd_read and rd_write */
{
    irqflags_t irq;
#ifdef RD_BLOCK_DEVICES
    bool error = false;
    uint16_t rv;

    flag;    /* unused */

    /* check device exists; do not allow writes to ROM */
    if(minor >= NUM_DEV_RD || (minor == RD_MINOR_ROM && rd_reverse)){
        error = true;
        rv = 0; /* keep the compiler quiet */
    }else{
        rd_src_address = dev_start[minor];

        if(rawflag || minor == RD_MINOR_PHYSMEM){ 
            /* rawflag == 1, userspace transfer */
            rd_dst_userspace = true;
            rd_dst_address = (uint16_t)udata.u_base;
            rd_src_address += udata.u_offset;
            rd_cpy_count = udata.u_count;
            if(minor == RD_MINOR_PHYSMEM) /* /dev/physmem is a character device -- return # bytes */
                rv = rd_cpy_count;
            else
                rv = rd_cpy_count >> 9;          /* block devices return # blocks */
        }else{
            /* rawflag == 0, kernel transfer */
            rd_dst_userspace = false;
            rd_dst_address = (uint16_t)&udata.u_buf->bf_data;
            rd_src_address += ((uint32_t)udata.u_buf->bf_blk << 9);
            rd_cpy_count = 512;
            rv = 1;
        }

        if(rd_src_address >= dev_limit[minor]){
            error = true;
        }
    }

    if(error){
#else
    flag;    /* unused */
    rawflag; /* unused */
    rd_dst_userspace = true;
    rd_dst_address = (uint16_t)udata.u_base;
    rd_src_address = udata.u_offset;
    rd_cpy_count = udata.u_count;

    if(minor != RD_MINOR_PHYSMEM || rd_src_address >= (1L << 20)){
#endif
        udata.u_error = EIO;
        return -1;
    }

    /* call rd_page_copy() with ints disabled to ensure another process does not stamp on the DMA control registers */
    irq = __hard_di();
    rd_page_copy();
    __hard_irqrestore(irq);

#ifdef RD_BLOCK_DEVICES
    return rv;
#else
    return rd_cpy_count;
#endif
}


int rd_open(uint8_t minor, uint16_t flags)
{
    flags; /* unused */

    switch(minor){
#if DEV_RD_RAM_PAGES > 0
        case RD_MINOR_RAM:
#endif
#if DEV_RD_ROM_PAGES > 0
        case RD_MINOR_ROM:
#endif
        case RD_MINOR_PHYSMEM:
            return 0;
        default:
            udata.u_error = ENXIO;
            return -1;
    }
}
