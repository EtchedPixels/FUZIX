/* Zeta SBC V2  memory driver
 *
 *     /dev/rd0      (block device)      RAM disk
 *     /dev/rd1      (block device)      ROM disk 
 *     /dev/physmem  (character device)  physical memory driver 
 *
 * 2017-01-03 William R Sowerbutts, based on RAM disk code by Sergey Kiselev
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#define DEVRD_PRIVATE
#include "devrd.h"

static const uint32_t dev_limit[NUM_DEV_RD] = {
    (uint32_t)(DEV_RD_RAM_PAGES + DEV_RD_RAM_START) << 14, /* block /dev/rd0: RAM */
    (uint32_t)(DEV_RD_ROM_PAGES + DEV_RD_ROM_START) << 14, /* block /dev/rd1: ROM */
    1L << 20                                               /* char  /dev/physmem: full 1MB address space */
};

static const uint32_t dev_start[NUM_DEV_RD] = {
    (uint32_t)DEV_RD_RAM_START << 14,                      /* block /dev/rd0: RAM */
    (uint32_t)DEV_RD_ROM_START << 14,                      /* block /dev/rd1: ROM */
    0                                                      /* char  /dev/physmem: full 1MB address space */
};

int rd_transfer(uint8_t minor, uint8_t rawflag, uint8_t flag) /* implements both rd_read and rd_write */
{
#if (DEV_RD_RAM_PAGES+DEV_RD_ROM_PAGES) == 0 /* this is not right -- kills /dev/physmem also */
    flag;    /* unused */
    minor;   /* unused */
    rawflag; /* unused */
    
    /* neither device is present -- just return an error */
    udata.u_error = ENXIO;
    return -1;
#else
    bool error = false;
    uint16_t count, rv, maxcpy;

    flag;    /* unused */

    /* check device exists; do not allow writes to ROM */
    if(minor >= NUM_DEV_RD || (minor == RD_MINOR_ROM && rd_reverse)){
        error = true;
        count = rv = 0; /* keep the compiler happy */
    }else{
        rd_src_address = dev_start[minor];

        if(rawflag || minor == RD_MINOR_PHYSMEM){ 
            /* rawflag == 1, userspace transfer */
            rd_dst_userspace = true;
            rd_dst_address = (uint16_t)udata.u_base;
            rd_src_address += udata.u_offset;
            count = udata.u_count;
            if(minor == RD_MINOR_PHYSMEM) /* /dev/physmem is a character device -- return # bytes */
                rv = count;
            else
                rv = count >> 9;          /* block devices return # blocks */
        }else{
            /* rawflag == 0, kernel transfer */
            rd_dst_userspace = false;
            rd_dst_address = (uint16_t)&udata.u_buf->bf_data;
            rd_src_address += ((uint32_t)udata.u_buf->bf_blk << 9);
            count = 512;
            rv = 1;
        }

        if(rd_src_address >= dev_limit[minor]){
            error = true;
        }
    }

    if(error){
        udata.u_error = EIO;
        return -1;
    }

    if(count == 1){
        /* fast path for single byte transfers */
        rd_cpy_count = 1;
        rd_page_copy();
    }else{
        while(true){
            rd_cpy_count = count;
            /* check if transfer will span over a 16KB bank boundary */
            maxcpy = ((uint16_t)rd_src_address) & 0x3FFF;
            if((rd_dst_address & 0x3FFF) > maxcpy)
                maxcpy = rd_dst_address & 0x3FFF;
            maxcpy = 0x4000 - maxcpy;
            if(rd_cpy_count > maxcpy)
                rd_cpy_count = maxcpy;
#ifdef DEBUG
            kprintf("rd_transfer: src=0x%lx, dst=0x%x(%s) reverse=%d count=%d\n",
                    rd_src_address, rd_dst_address, rd_dst_userspace?"user":"kern",
                    rd_reverse, rd_cpy_count);
#endif
            rd_page_copy();

            count -= rd_cpy_count;
            if(!count)
                break;
            rd_dst_address += rd_cpy_count;
            rd_src_address += rd_cpy_count;
        }
    }

    return rv;
#endif
}


int rd_open(uint8_t minor, uint16_t flags)
{
    flags; /* unused */

#ifdef DEBUG
    kprintf("rd_open(%d)\n", minor);
#endif

#if (DEV_RD_RAM_PAGES+DEV_RD_ROM_PAGES) == 0
    minor; /* unused */

    udata.u_error = EIO;
    return -1;
#else
    switch(minor){
#if DEV_RD_RAM_PAGES > 0
        case RD_MINOR_RAM:
#endif
#if DEV_RD_ROM_PAGES > 0
        case RD_MINOR_ROM:
#endif
#ifdef CONFIG_DEV_PHYSMEM
        case RD_MINOR_PHYSMEM:
#endif
#ifdef DEBUG
    kprintf("rd_open(%d): ok!\n", minor);
#endif
            return 0;
        default:
            udata.u_error = ENXIO;
            return -1;
    }
#endif
}
