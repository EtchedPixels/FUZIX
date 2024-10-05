/* 
 * PZ1 block device driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <ports.h>
#include <tinydisk.h>

#define BLOCK_SIZE 512

/* Asm transfer routines in common space */
extern uint8_t hd_data_in(uint8_t *ptr);
extern uint8_t hd_data_out(uint8_t *ptr);

int hd_xfer(uint8_t dev, bool is_read, uint32_t lba, uint8_t * dptr)
{
	if ((dev >> 4) != 0)
		return 0;

	/* seek to wanted block using 24-bit LBA */
	io_disk_prm0 = (uint8_t)(lba & 255);
	io_disk_prm1 = (uint8_t)(lba >> 8);
	io_disk_prm2 = (uint8_t)(lba >> 16);
	io_disk_cmd = DISK_CMD_SEEK;
	if (io_disk_status != DISK_STATUS_OK)
		return 0;

	/* transfer a block */
	if (is_read) {
		if (hd_data_in(dptr))
			return 0; /* failure to read complete block */
	} else {
		if (hd_data_out(dptr))
			return 0; /* failure to write complete block */
	}
	/* all is well! */
	return 1;
}

void hd_init(void)
{
	td_register(0, hd_xfer, td_ioctl_none, 1);
}
