/* 
 * CPC standard RAM bank memory expansions ramdisc driver, based on platform zxdiv48.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#if defined EXTENDED_RAM_512 || defined EXTENDED_RAM_1024
#include <devrd.h>


static int rd_transfer(uint8_t is_read, uint8_t rawflag)
{
    nblock = udata.u_nblock; 
    block = udata.u_block;
    rd_dptr = udata.u_dptr;
    rd_wr = is_read;
    uint16_t swap_bank_long;
    uint8_t ct = 0;
    
    #ifdef DEBUG
        kprintf("u_dptr %p Block %u u_nblock %u rd_wr %u\n",udata.u_dptr, udata.u_block, udata.u_nblock, rd_wr);
    #endif

    /* It's a disk but only for swapping (and rd_io isn't general purpose) */
    if (((block + nblock) > (TOTAL_SWAP_BLOCKS - 1)) || (rawflag == 1))  {
        udata.u_error = EIO;
            kprintf("dev_rd_EIO");
        return -1;
    }
    
    /* udata could change under us so keep variables privately */
    while (ct < nblock) {
        swap_bank_long = (block >> 5);
        swap_bank_long = swap_bank_long + 196 + (((swap_bank_long + 8) / 4) * 4); /*Convert bank number to Register MMR value 
                                                      *See https://www.cpcwiki.eu/index.php/Gate_Array#Register_MMR_.28RAM_memory_mapping.29*/
        if (swap_bank_long > 255){
            rd_swap_bank = swap_bank_long - 64;
            rd_swap_mem_port_h = 0x7e;
        }
        else{
            rd_swap_bank = swap_bank_long;
            rd_swap_mem_port_h = 0x7f;
        }
        rd_proc_bank = ((uint16_t)rd_dptr / 0x4000) + 0xc4;
        rd_swap_bank_addr = ((block & 31) << BLKSHIFT) + 0x4000;
        rd_proc_bank_addr = ((uint16_t)rd_dptr & 0x3fff) + 0x4000;
        
        #ifdef DEBUG
            if (nblock == 1)
                kprintf("swap_bank %p swap_addr %p proc_bank %p proc_addr %p count %u\n", rd_swap_bank, rd_swap_bank_addr, rd_proc_bank, rd_proc_bank_addr, ct);
        #endif
        rd_io();        
        block++;
        rd_dptr += BLKSIZE;
        ct++;
    }
    return ct << BLKSHIFT; /*Total bytes transferred*/
}

int rd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return rd_transfer(true, rawflag);
}

int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;minor;
    return rd_transfer(false, rawflag);
}
#endif
