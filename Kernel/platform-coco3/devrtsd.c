/* 

   Roger Taylor's CoCo On a Chip SD card driver
   (for use with firmware > 031618)
   
   Fixme: these routines need to fail gracefully.
*/


#include <kernel.h>
#include <kdata.h>
#include <blkdev.h>
#include <mbr.h>
#include <devrtsd.h>
#include <printf.h>



#define sdc_data   *((volatile uint8_t *) 0xff70)
#define sdc_status *((volatile uint8_t *) 0xff71)
#define    SDC_ST_RTS   0x80
#define    SDC_ST_BUSY  0x40
#define    SDC_ST_SBUSY 0x20
#define    SDC_ST_VALID 0x10
#define    SDC_ST_MOUNT 0x08
#define    SDC_ST_CLASS 0x04
#define    SDC_ST_WBUSY 0x01
#define sdc_command *((volatile uint8_t *) 0xff71)
#define    SDC_CMD_READ   0x00
#define    SDC_CMD_WRITE  0x10
#define    SDC_CMD_RESET  0x30
#define    SDC_CMD_MOUNT  0x20
#define    SDC_CMD_DIRECT 0xC0
#define    SDC_CMD_IMAGE  0x80
#define    SDC_CMD_GMOUNT 0x40
#define sdc_lsn_high *((volatile uint8_t *) 0xff72)
#define sdc_lsn_mid  *((volatile uint8_t *) 0xff73)
#define sdc_lsn_low  *((volatile uint8_t *) 0xff74)

typedef void (*sdc_transfer_function_t)(unsigned char *addr);


/* blkdev method: flush drive */
int devrtsd_flush(void)
{
	return 0;
}

/* blkdev method: transfer sectors */
uint8_t devrtsd_transfer(void)
{
	int i = 2;
	uint8_t *ptr;
	sdc_transfer_function_t fptr;
	uint8_t cmd;

	/* convert to 256 byte sector LBA */
	blk_op.lba *= 2;

	if (blk_op.is_read) {
		cmd = SDC_CMD_READ;
		fptr = devrtsd_read;
	} else {
		cmd = SDC_CMD_WRITE;
		fptr = devrtsd_write;
	}

	cmd |= blk_op.blkdev->driver_data;
	ptr = ((uint8_t *) (&blk_op.lba)) + 1;
	while (i--) {
		/* set lsn */
		sdc_lsn_high = ptr[0];
		sdc_lsn_mid = ptr[1];
		sdc_lsn_low = ptr[2];
		/* wait for ready */
		while (sdc_status & (SDC_ST_BUSY | SDC_ST_SBUSY));
		/* send command */
		sdc_command = cmd;
		/* xfer data */
		fptr(blk_op.addr);
		/* wait for ready */
		while (sdc_status & (SDC_ST_BUSY | SDC_ST_SBUSY));
		blk_op.lba++;
		blk_op.addr += 256;
	}
	return 1;
}

__attribute__((section(".discard")))
/* blkdev method: initalize device */
void devrtsd_init(void)
{
	blkdev_t *blk;
	int i;

	kputs("RTSD: ");
	/* fixme: add some device checking/reporting here */
	for (i = 0; i < 4; i++) {
		blk = blkdev_alloc();
		blk->driver_data = i;
		blk->transfer = devrtsd_transfer;
		blk->flush = devrtsd_flush;
		/* assume max 24 bit size? (how big are images?) */
		blk->drive_lba_count = 16777216;
	}
	kputs("Ok.\n");
}
