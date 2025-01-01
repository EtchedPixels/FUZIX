/* Zeta SBC V2  memory driver
 *
 * 2017-01-03 William R Sowerbutts, based on RAM disk code by Sergey Kiselev
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#define DEVRD_PRIVATE
#include "devrd.h"

void rd_page_copy(void); // devrd_zeta2_hw.s

void rd_plt_copy(void)
{
    uint16_t ocount, count, maxcpy;

    ocount = count = rd_cpy_count;

    while(true){
        /* ensure transfer will not span a 16KB bank boundary */
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
        rd_cpy_count = count;
    }

    rd_cpy_count = ocount;
}
