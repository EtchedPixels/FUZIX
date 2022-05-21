#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devscsi.h>
#include <blkdev.h>
#include <dma.h>
#include <acsi.h>

/*
 *	ACSI Hard disk interface. Based upon EmuTOS
 *
 *	ACSI is a SASI/SCSI like interface. The drives only speak SASI style
 *	6 byte commands but we are at least blessed with a DMA interface for
 *	this even if it's a bit of a pain to drive except for block I/O.
 *
 *	There is a hack (not implemented yet) where full SCSI commands can
 *	be sent to some drive convertors by proceeding the longer commands
 *	with 0x1F.
 */


union acsidma {
	volatile uint32_t datacontrol;
	struct {
		volatile uint16_t data;
		volatile uint16_t control;
	} s;
};

#define ACSIDMA ((union acsidma *) 0xFF8604)

static timer_t acsi_next;

#define ACSI_CMDWAIT	TICKSPERSEC/100
#define ACSI_CMDBYTE	TICKSPERSEC/10
#define ACSI_OP		TICKSPERSEC


void acsi_select(void)
{
	/* We are not allowed to issue the next command too soon */
	while (!timer_expired(acsi_next))
		plt_idle();

	/* Claim the DMA */
	dma_lock();
}

void acsi_clear(void)
{
	/* Hand back the DMAC */
	ACSIDMA->s.control = DMA_FDC;
	dma_unlock();

	/* Remember when we can next issue a command */
	acsi_next = set_timer_duration(ACSI_CMDWAIT);
}

/* Send byte and *next* control */
static void dma_send_byte(uint8_t c, uint16_t control)
{
	ACSIDMA->datacontrol = ((uint32_t) c) << 16 | control;
}

static void delay(void)
{
	/* Wait 15us : FIXME */
}

/* Flush FIFO and set control */
static void hdc_start_dma(uint16_t control)
{
	control |= DMA_SCREG;
	ACSIDMA->s.control = control ^ DMA_WRBIT;
	delay();
	ACSIDMA->s.control = control;
	delay();
}

/*
 *	The SCSI midlayer wishes to send a command out
 */
uint8_t acsi_execute(uint8_t * cmd, uint8_t cmdlen, uint16_t len)
{
	uint8_t repeat = 0;
	uint8_t st;
	uint8_t control;
	int i;

	/* FIXME: add ICD hacks */

	if (len && !blk_op.is_read)
		/* Flush cache */ ;

	/* No banking complexities for us. Some day we may have to address non DMA
	   memory on big machines however FIXME */
	if (len)
		set_dma_addr(blk_op.addr);
	control = DMA_FDC | DMA_HDC;
	if (!blk_op.is_read)
		control |= DMA_WRBIT;
	hdc_start_dma(control);
	ACSIDMA->s.data = (len + 511) >> 9;

	/* Repeat if neeed to drive through fifo.. ick */
	if (len & 511) {
		/* FIXME: add correct logic here */
		repeat = 1;
	}

	do {
		uint8_t *p = cmd;

		/* We are not allowed to issue the next command too soon */
		while (!timer_expired(acsi_next))
			plt_idle();

		/* Write the command */
		ACSIDMA->s.control = control;	/* SCSI control */
		control |= DMA_A0;	/* Remaining bytes need this */
		for (i = 0; i < cmdlen - 1; i++) {
			dma_send_byte(*p++, control);
			/* Allow 100ms */
			if (dma_wait(ACSI_CMDBYTE))
				return -1;
		}
		/* Command final byte */
		dma_send_byte(*p, control & 0xFF00);

		/* Now wait for command to run - can take a second worst case */
		st = dma_wait(ACSI_OP);
		/* Remember when we can issue the next command */
		acsi_next = set_timer_duration(ACSI_CMDWAIT);
		if (st)
			return -1;
		/* Command issued */
		ACSIDMA->s.control = control & ~DMA_WRBIT;
		st = ACSIDMA->s.data & 0xff;
		if (st)
			break;
		control &= ~DMA_A0;
	} while (repeat--);
	/* And done */
	if (len && blk_op.is_read)
		/* Flush cache */ ;
	return 0;
}

uint8_t acsi_transfer(void)
{
	uint8_t cmd[11];
	unsigned short cmdlen;
	unsigned short count;
	unsigned short dev;
	uint8_t blocks = blk_op.nblock;

	if (blk_op.nblock > 255)
		blocks = 255;

	/* FIXME: lun handling */
	dev = blk_op.blkdev->driver_data & DRIVE_NR_MASK;
	/* When we can we issue ACSI commands. When we are out of range we try
	   SCSI. All ACSI devices are small enough only ACSI will hit them */
	if (blk_op.lba < 0x200000) {
		cmd[0] = blk_op.is_read ? 0x08 : 0x0A;
		cmd[1] = (blk_op.lba >> 16) & 0x1f;
		cmd[2] = blk_op.lba >> 8;
		cmd[3] = blk_op.lba;
		cmd[4] = blocks;
		cmd[5] = 0x00;
		cmdlen = 6;
	} else {
		/* Framed SCSI READ_10/WRITE_10 */
		cmd[0] = 0x1F;	/* SCSI follows, lun bits also go here */
		cmd[1] = blk_op.is_read ? 0x28 : 0x2A;
		cmd[2] = 0x00;	/* LUN */
		cmd[3] = blk_op.lba >> 24;
		cmd[4] = blk_op.lba >> 16;
		cmd[5] = blk_op.lba >> 8;
		cmd[6] = blk_op.lba;
		cmd[7] = 0x00;
		cmd[8] = 0x00;	/* Blocks upper 8. Need to fix blkdev.c before */
		cmd[9] = blocks;	/* we can do > 255 blocks per I/O */
		cmd[10] = 0x00;
		cmdlen = 11;
	}

	acsi_select();
	/* Should we drop back to single block commands on an error and try to
	   do them all one by one ? */
	for (count = 0; count < 5; count++) {
		if (acsi_execute(cmd, cmdlen, blocks << 9) == 0) {
			acsi_clear();
			return blocks;
		}
	}
	acsi_clear();
	kprintf("acsi: device %d failed command %x for block %lx\n", dev, cmd[0], blk_op.lba);
	return 0;
}

static unsigned int acsi_probe(uint8_t dev, uint8_t lun)
{
	static uint8_t cdb[6];	/* all zeros is TUR */
	unsigned int st;
	blkdev_t *blk;

	acsi_select();
	st = acsi_execute(cdb, 6, 0);
	acsi_clear();

	if (st)
		return st;

	/* TODO: do we need to check type and sector size etc on ACSI - prob not */

	/* Add it */
	blk = blkdev_alloc();
	blk->transfer = acsi_transfer;
	//blk->flush = acsi_flush;
	blk->driver_data = dev;	/* FIXME lun */
	//blk->drive_lba_count = acsi_capcity(dev, lun);
	blkdev_scan(blk, SWAPSCAN);
	return st;
}

void acsi_init(void)
{
	unsigned int dev, lun;

	acsi_next = set_timer_duration(0);
	for (dev = 0; dev < 8; dev++) {
		for (lun = 0; lun < 8; lun++) {
			if (acsi_probe(dev, lun) == 0) {
			} else if (lun == 0)
				break;
		}
	}
}
