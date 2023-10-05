/* 
 * PZ1 block device driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <ports.h>

#define BLOCK_SIZE 512

int hd_open(uint_fast8_t minor, uint16_t flag)
{
        used(flag);
        if(minor != 0) {
                udata.u_error = ENODEV;
                return -1;
        }
        return 0;
}

/* Asm transfer routines in common space */
extern uint8_t hd_data_in(uint8_t *ptr);
extern uint8_t hd_data_out(uint8_t *ptr);

uint8_t hd_mode;

/* The main disk read/write loop */
int hd_rw(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
        uint8_t *dptr;
        uint16_t n = 0;
    
        used(minor); 
        used(flag);

        if (rawflag == 1) { 
                if (d_blkoff(BLKSHIFT))
                        return -1;
        }
        /* No swap support yet. In the swap case we do I/O to or from the page
           in swappage */
        if (rawflag == 2)
                return -1;
    
        // seek to wanted block
        port_fs_prm0 = udata.u_block & 255;
        port_fs_prm1 = udata.u_block >> 8;
        port_fs_cmd = FS_CMD_SEEK;

        if (port_fs_status != FS_STATUS_OK)
                return -1;
    
        dptr = udata.u_dptr;
        hd_mode = rawflag;

        // kprintf("I/O %u sector(s) from %u to address %u\n", udata.u_nblock, udata.u_block, udata.u_dptr);

        while (udata.u_nblock--) {
                if (is_read) {
                        if (hd_data_in(dptr))
                                break;
                } else if (hd_data_out(dptr))
                        break;
                dptr += BLKSIZE;
                n += BLKSIZE;
        }
        // kprintf("I/O completd %u\n", n);
        return n;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
        used(flag);
	return hd_rw(minor, true, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
        used(flag);
	return hd_rw(minor, false, rawflag);
}
