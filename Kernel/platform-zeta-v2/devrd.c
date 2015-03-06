/* Zeta SBC V2 RAM disk driver 
 *
 * Implements a single RAM disk DEV_RD_PAGES size and
 * starting from DEV_RD_START page
 *
 * */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

extern uint8_t src_page;	/* source page number */
extern uint8_t dst_page;	/* destination page number */
extern uint16_t src_offset;	/* offset of the data in the source page */
extern uint16_t dst_offset;	/* offset of the data in the destination page */
extern uint16_t cpy_count;	/* data transfer length */
extern uint8_t kernel_pages[];	/* kernel's page table */

int ramdisk_transfer(bool is_read, uint8_t minor, uint8_t rawflag);
int page_copy(void);		/* assembler code */

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
	uint32_t rd_addr;
	uint16_t buffer_addr;
	usize_t xfer_count;

	if (minor >= NUM_DEV_RD) {
		udata.u_error = ENXIO;
		return -1;
	}

	if (rawflag) { /* rawflag == 1, read to or write from user space */
		xfer_count = udata.u_count;
		buffer_addr = (uint16_t) udata.u_base;
		block = udata.u_offset >> 9;
		block_xfer = xfer_count >> 9;
	} else { /* rawflag == 0, read to or write from kernel space */
		xfer_count = 512;
		buffer_addr = (uint16_t) udata.u_buf->bf_data;
		block = udata.u_buf->bf_blk;
		block_xfer = 1;
	}

	if (block > (DEV_RD_PAGES * 16 * 2)) { /* block beyond RAM disk end? */
		udata.u_error = EIO;
		return -1;
	}

	/* calculate physical address of the RAM drive data */
	/* rd_addr = block * 512 + DEV_RD_START * 16K; */
	rd_addr = ((unsigned long) block << 9) + DEV_RD_START * 16384UL;
	while (xfer_count > 0) {
		if (is_read) {
			/* RAM disk page number = rd_addr / 16K */
			src_page = rd_addr >> 14;
			/* offset within RAM disk page */
			src_offset = rd_addr & 0x3FFF;
			/* destination page number */
			if (rawflag)
				dst_page = ((uint8_t *) &udata.u_page)[buffer_addr >> 14];
			else
				dst_page = kernel_pages[buffer_addr >> 14];
			/* offset in the destination page */
			dst_offset = buffer_addr & 0x3FFF;
		} else {
			/* source page number */
			if (rawflag)
				src_page = ((uint8_t *) &udata.u_page)[buffer_addr >> 14];
			else
				src_page = kernel_pages[buffer_addr >> 14];
			/* offset in the source page */
			src_offset = buffer_addr & 0x3FFF;
			/* RAM disk page number = rd_addr / 16K */
			dst_page = rd_addr >> 14;
			/* offset within RAM disk page */
			dst_offset = rd_addr & 0x3FFF;
		}
		cpy_count = xfer_count;
		if (cpy_count > 16384 - src_offset)
			cpy_count = 16384 - src_offset;
		if (cpy_count > 16384 - dst_offset)
			cpy_count = 16384 - dst_offset;
#ifdef DEBUG
		kprintf("page_cpy(src_page=%x, src_offset=%x, dst_page=%x, dst_offset=%x, cpy_count=%x)\n", src_page, src_offset, dst_page, dst_offset, cpy_count);
#endif
		page_copy();
		xfer_count -= cpy_count;
		buffer_addr += cpy_count;
		rd_addr += cpy_count;
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
