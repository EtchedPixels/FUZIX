/* socz80 RAM disk driver 
 *
 * Implements two RAM disks in the top 4MB of RAM, starting at 4MB and 6MB.
 * Each disk is 2MB in size.
 *
 * */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

volatile char   *rd_dptr;          /* pointer to data buffer */
volatile int     rd_dlen;          /* data transfer length */
volatile uint16_t  rd_address;     /* disk address (in 256-byte chunks) */

int ramdisk_transfer(bool is_read, uint8_t minor, uint8_t rawflag);
int ramdisk_read(void); // assembler
int ramdisk_write(void); // assembler

int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return ramdisk_transfer(true, minor, rawflag);
}

int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return ramdisk_transfer(false, minor, rawflag);
}

int ramdisk_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;     /* r/w return value (number of 512 byte blocks transferred) */
    char *dptr;
    int dlen;
    uint16_t addr;

    if(rawflag){
        dlen = udata.u_count;
        dptr = udata.u_base;
        block = udata.u_offset >> 9;
        block_xfer = dlen >> 9;
    }else{ /* rawflag == 0 */
        dlen = 512;
        dptr = udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        block_xfer = 1;
    }

    if(block > (2 * 1024 * 2)){ /* > 2MB? */
        udata.u_error = EIO;
        return -1;
    }

    switch(minor){
        case 0: 
            block += (4 * 1024 * 2); /* rd0 at 4MB */ 
            break;
        case 1: 
            block += (6 * 1024 * 2); /* rd1 at 6MB */
            break;
        default:
            udata.u_error = ENXIO;
            return -1;
    }

    /* compute address in 256-byte chunks */
    addr = block << 1;

    // kprintf("ramdisk_transfer(%s, %d, block=0x%04x, dptr=0x%04x, dlen=0x%04x)",
    //         is_read?"read":"write", minor, block, dptr, dlen);

    /* FIXME Should be able to avoid the __critical once bank switching is fixed */
    __critical {
        rd_dlen = dlen;
        rd_dptr = dptr;
        rd_address = addr;
        if(is_read)
            ramdisk_read();
        else
            ramdisk_write();
    }

    return block_xfer;
}


int rd_open(uint8_t minor)
{
    if(minor < NUM_DEV_RD){
        return 0;
    } else {
        udata.u_error = EIO;
        return -1;
    }
}
