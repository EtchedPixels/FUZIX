/*
 *	The MicroBee 128 shipped with a WD1002-05 and various small drives.
 *
 *	The driver is based on the TRS80 WD1010 driver. The 1002-05 is a 1010
 *	with additional glue chips that also manage and front up a floppy
 *	controller for us including the DMA management. It's actually a better
 *	floppy controller than almost anything that followed it !
 *
 *	Minor is arranged as
 *
 * 	S		slot (0 or 1 via port 0x58)
 *	DDD		drive identifier
 *			0-2	hdc
 *			3-6	fdc
 *	MMMM		0000 (reserved for partitioning on hd)
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <ubee.h>
#include <blkdev.h>

__sfr __at 0x40 hd_data;
__sfr __at 0x41 hd_precomp;	/* W/O */
__sfr __at 0x41 hd_err;		/* R/O */
__sfr __at 0x42 hd_seccnt;
__sfr __at 0x43 hd_secnum;
__sfr __at 0x44 hd_cyllo;
__sfr __at 0x45 hd_cylhi;
__sfr __at 0x46 hd_sdh;
__sfr __at 0x47 hd_status;	/* R/O */
__sfr __at 0x47 hd_cmd;
__sfr __at 0x48 hd_fdcside;	/* Side select for FDC */
__sfr __at 0x58 fdc_devsel;	/* Whch controller to use */

#define HDCMD_RESTORE	0x10
#define HDCMD_READ	0x20
#define HDCMD_WRITE	0x30
#define HDCMD_VERIFY	0x40	/* Not on the 1010 later only */
#define HDCMD_FORMAT	0x50
#define HDCMD_INIT	0x60	/* Ditto */
#define HDCMD_SEEK	0x70

#define RATE_2MS	0x04	/* 2ms step rate for hd (conservative) */
#define RATE_6MS	0x06	/* 6ms step for floppy (3ms is probably fine
				   for most) */

#define HDSDH_ECC256		0x80

/* Used by the asm helpers */
uint8_t hd_page;

/* Seek and restore low 4 bits are the step rate which we need to sort out
   FIXME */

#define MAX_HD	3	/* devsel 3 means floppy */
#define MAX_FDC	4

/* Standard formats for Microbee:
	40 x 2 x 10 x 512 double sided 5.25
	80 x 2 x 10 x 512 single sided 3.5
	80 x 2 x 10 x 512 double sided 3.5

	For now just use double sided : FIXME */
static int spt[7] = { 32, 32, 32, 10, 10, 10, 10 };
static int heads[7] = { 4, 4, 4, 2, 2, 2, 2 };

/* Wait for the drive to show ready */
static uint8_t hd_waitready(void)
{
	uint8_t st;
	uint16_t tick = 0;
	do {
		st = hd_status;
		tick++;
	} while (!(st & 0x40) && tick);
	return st;
}

/* Wait for the controller to show idle */
static uint8_t hd_waitidle(void)
{
	uint8_t st;
	uint16_t tick = 0;
	do {
		st = hd_status;
		tick++;
	} while ((st & 0x80) && tick);
	return st;
}

/*
 *	Transfer for routine for the WD1010 (ST-506) interface.
 */

static uint8_t hd_transfer_sector(void)
{
	uint8_t drive = blk_op.blkdev->driver_data & 0x0F;
	uint8_t sel = !!(drive & 0x08);

	uint32_t b;
	uint8_t sector, head;
	uint8_t err;
	uint8_t tries;

	drive &= 0x07;

	b = blk_op.lba / spt[drive];
	sector = blk_op.lba % spt[drive];
	head = b % heads[drive];
	b /= heads[drive];

	fdc_devsel = sel;

	if (drive < MAX_HD) {
		/* ECC, 512 bytes, head and drive */
		hd_sdh = 0xA0 | head | (drive << 3);
	} else {
		/* Floppy setup */
		hd_fdcside = head;
		/* CRC, 512 bytes, head (0/1), FDC unit, fdc number */
		hd_sdh = 0x38 | head | ((drive - MAX_HD) << 1);
	}
	hd_secnum = sector + 1;

	/* cylinder bits */
	hd_cyllo = b;
	hd_cylhi = b >> 8;

	for (tries = 0; tries < 4; tries++) {
		/* issue the command */
		hd_cmd = blk_op.is_read ? HDCMD_READ : HDCMD_WRITE;
		err = 0;
		/* Wait for busy to drop if reading before xfer */
		if (blk_op.is_read) {
			hd_waitidle();
			hd_xfer_in();
			err = hd_status;
		} else {
			hd_xfer_out();
			/* Wait for busy to drop after xfer if writing */
			err = hd_waitidle();
			/* Ready, no error, not busy ? */
		}
		if ((err & 0xC1) == 0x40)
			return 1;

		kprintf("hd%d: err %x\n", drive, err);

		if (tries > 1) {
			hd_cmd = HDCMD_RESTORE | RATE_6MS;
			if (hd_waitready() & 1)
				kprintf("hd%d: restore error %x\n", drive, err);
		}
	}
	if (err & 1)
		kprintf("hd%d: error %x\n", drive, hd_err);
	else
		kprintf("hd%d: status %x\n", drive, err);
	return 0;
}

static void hd_init_drive(uint8_t drive)
{
	blkdev_t *blk;
	uint8_t sel = !!(drive & 0x08);

	/* Only probe controllers that are present */
	if (disk_type[sel] != DISK_TYPE_HDC)
		return;

	fdc_devsel = sel;

	drive &= 0x07;

	if (drive < MAX_HD) {
		hd_sdh = 0xA0 | (drive << 3);
		hd_cmd = HDCMD_RESTORE | RATE_2MS;
	} else {
		hd_sdh = 0x38 | ((drive - MAX_HD) << 1);
		hd_cmd = HDCMD_RESTORE | RATE_6MS;
	}
	if (hd_waitready() & 1) {
		if ((hd_err & 0x12) == 0x12)
			return;
	}
	/* Found */
	blk = blkdev_alloc();
	if (blk == NULL) {
		kputs("hd: too many drives.\n");
		return;
	}
	blk->transfer = hd_transfer_sector;
	blk->flush = NULL;
	blk->driver_data = drive | (sel << 3);
	blk->drive_lba_count = 0xFFFFFF;
	/* No partitions on floppy disks thank you */
	if (drive < MAX_HD)
		blkdev_scan(blk, SWAPSCAN);
}

void hd_init(void)
{
	uint8_t d;
	/* 0 1 2 are hard disks, 3-7 floppy appearing as hd */
	/* 8+ because we also need to scan the other slot if present */
	for (d = 0; d < 15; d++)
		hd_init_drive(d);
}

COMMON_MEMORY

void hd_xfer_in(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #0x40	                    ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapin
not_swapin:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always                ; map user memory first if required
swapin:
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void hd_xfer_out(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #0x40	                    ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapout
not_swapout:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always                ; else map user memory first if required
swapout:
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
