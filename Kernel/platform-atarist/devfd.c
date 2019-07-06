/*
 *	This is heavily based upon EmuTOS
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devfd.h>
#include <dma.h>
#include <psg.h>

#define MAX_FD	2

static uint8_t present[MAX_FD];		/* Is this drive present */
static uint8_t tracknum[MAX_FD];	/* What track is the head on */
static uint8_t dsided[MAX_FD] = { 1, 1 };	/* For now FIXME */
static uint8_t step[MAX_FD] = { 3, 3 };	/* 3ms happens to be the value 3 */

static uint8_t deselected = 1;

#define FDC_CS	(DMA_FDC)
#define FDC_TR  (DMA_FDC | DMA_A0)
#define FDC_SR  (DMA_FDC | DMA_A1)
#define FDC_DR  (DMA_FDC | DMA_A1 | DMA_A0)

#define FDC_RESTORE	0x00
#define FDC_SEEK	0x10
#define FDC_STEP	0x20
#define FDC_STEPI	0x40
#define FDC_STEPO	0x60
#define FDC_READ	0x80
#define FDC_WRITE	0xA0
#define FDC_READID	0xC0
#define FDC_IRUPT	0xD0
#define FDC_READTR	0xE0
#define FDC_WRITETR	0xF0

#define FDC_HBIT	0x08	/* Don't turn on motor */

#define FDC_BUSY	0x01
#define FDC_DRQ		0x02
#define FDC_LOSTDAT	0x04
#define FDC_TRACK0	0x04
#define FDC_CRCERR	0x08
#define FDC_RNF		0x10
#define FDC_RT_SU	0x20
#define FDC_WRI_PRO	0x40
#define FDC_MOTORON	0x80

/*
 *	Accesses and transfers via the DMA engine. Some of this might
 *	belong in its own place
 */

static void delay(void)
{
	/* TODO */
}

static void flush_cache_range(void *p, size_t len)
{
	/* TODO */
}

/*
 *	Read an fd register
 */
static uint16_t fd_get_reg(uint16_t reg)
{
	uint16_t ret;

	DMA->control = reg;
	delay();
	ret = DMA->data;
	delay();
	return ret;
}

/*
 *	Write to an FD register
 */
static void fd_set_reg(uint16_t reg, uint16_t value)
{
	DMA->control = reg;
	delay();
	DMA->data = value;
	delay();
}

/*
 *	Trigger a DMA read. The WRBIT toggle clears anything in the FIFO
 */
static void fd_start_dma_read(uint16_t count)
{
	DMA->control = DMA_SCREG | DMA_FDC;
	DMA->control = DMA_SCREG | DMA_FDC | DMA_WRBIT;
	DMA->control = DMA_SCREG | DMA_FDC;
	DMA->data = count;
}

/*
 *	Trigger a DMA write. The WRBIT toggle clears anything in the FIFO
 */
static void fd_start_dma_write(uint16_t count)
{
	DMA->control = DMA_SCREG | DMA_FDC | DMA_WRBIT;
	DMA->control = DMA_SCREG | DMA_FDC;
	DMA->control = DMA_SCREG | DMA_FDC | DMA_WRBIT;
	DMA->data = count;
}

/*
 *	Wait for the floppy controller to respond and show up on the MFP
 *	GPIO.
 */

static int fd_wait(void)
{
	return dma_wait(3 * TICKSPERSEC);
}

/*
 *	Call every 0.1 sec or so to deal with motor off deselection
 */
void fd_event(void)
{
	irqflags_t irq;
	uint16_t status;

	if (deselected || dma_is_locked())
		return;
	status = fd_get_reg(FDC_CS);
	if (status & FDC_MOTORON)
		return;
	irq = di();
	PSG->control = PSG_PORT_A;
	PSG->data = PSG->control | 6;
	deselected = 1;
	irqrestore(irq);
}

/*
 *	Set the drive and side
 */
static void fd_set_side(uint8_t minor, uint8_t side)
{
	/* Someone was saving gates.. the control is via the PSG */
	/* Protect from sound interrupts in future */
	/* Probably want a PSG port helper and PSG file */
	irqflags_t irq = di();
	uint16_t a;

	PSG->control = PSG_PORT_A;
	a = PSG->control & 0xF8;
	if (minor)
		a |= 2;
	else
		a |= 4;
	if (!side)
		a |= 1;
	PSG->data = a;
	irqrestore(irq);
	deselected = 0;
}

/*
 *	Restore back to track 0 if we are going there or if the drive is
 *	lost having had a seek fail
 */
static int fd_restore(uint8_t minor)
{
	fd_set_reg(FDC_CS, FDC_RESTORE | step[minor]);
	if (fd_wait()) {
		tracknum[minor] = 255;
		return -1;
	}
	tracknum[minor] = 0;
	return 0;
}

/*
 *	Get the head back on track
 */
static int fd_set_track(uint8_t minor, uint8_t track)
{
	if (tracknum[minor] == 255 && fd_restore(minor))
		return -1;

	if (tracknum[minor] == track)
		return 0;
	/* Restore for track 0, or if we don't know where we are */
	if (track == 0)
		return fd_restore(minor);
	else {
		fd_set_reg(FDC_DR, track);
		fd_set_reg(FDC_CS, FDC_SEEK | step[minor]);
		if (fd_wait()) {
			tracknum[minor] = 255;	/* Force a reset */
			return -1;
		}
		tracknum[minor] = track;
	}
	return 0;
}

/*
 *	Transfer one sector
 */

static int fd_xfer_sector(uint8_t minor, uint8_t is_read)
{
	uint8_t sector = (udata.u_block % 9) + 1;
	uint8_t track = (udata.u_block / 9);
	uint8_t side = 0;
	uint8_t status;
	int tries = 0;

	/* We don't support double stepping */

	/* If we are double sided then we need to halve the track and switch side */
	if (dsided[minor]) {
		side = track & 1;
		track >>= 1;
	}

	dma_lock();

	/* Get into position */
	fd_set_side(minor, side);
	if (fd_set_track(minor, track)) {
		dma_unlock();
		return -1;
	}

	while (tries++ < 4) {
		if (tries > 2) {
			/* We could try a jiggle with try 3 and a restore with 4 */
			fd_restore(minor);
			/* Consider switching step rate */
			fd_set_track(minor, track);
		}
		fd_set_reg(FDC_SR, sector);
		set_dma_addr(udata.u_dptr);
		if (is_read) {
			fd_start_dma_read(1);
			fd_set_reg(FDC_CS, FDC_READ);
		} else {
			flush_cache_range(udata.u_dptr, 512);
			fd_start_dma_write(1);
			fd_set_reg(FDC_CS | DMA_WRBIT, FDC_WRITE);
		}
		if (fd_wait())
			continue;

		status = get_dma_status();
		if (!(status & DMA_OK))
			continue;

		status = fd_get_reg(FDC_SR);
		if (!is_read && (status & FDC_WRI_PRO)) {
			dma_unlock();
			udata.u_error = EROFS;
			return -1;
		}
		if (status & (FDC_RNF | FDC_CRCERR | FDC_LOSTDAT)) {
			kprintf("fd%d: error %x\n", minor, status);
			continue;
		}
		/* Whoopeee it worked */
		dma_unlock();
		if (is_read)
			flush_cache_range(udata.u_dptr, 512);
		return 0;
	}
	dma_unlock();
	udata.u_error = EIO;
	return -1;
}


/*
 *	Transfer multiple sectors as needed
 */
static int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	unsigned int ct = 0;
	static uint8_t last = 255;

	/* Flat model so no banking complications */

	if (rawflag == 1 && d_blkoff(BLKSHIFT))
		return -1;
	/* FIXME: swap */
	else if (rawflag == 2) {
		udata.u_error = EIO;
		return -1;
	}
	if (((uint32_t)udata.u_dptr) & 1) {
		udata.u_error = EINVAL;
		return -1;
	}

	if (minor != last) {
		last = minor;
		fd_set_reg(FDC_TR, tracknum[minor]);
	}

	while (ct < udata.u_nblock) {
		if (fd_xfer_sector(minor, is_read))
			return -1;
		udata.u_block++;
		udata.u_dptr += 512;
		ct++;
	}
	return ct;
}

/*
 *	Called when a floppy device is opened. We don't do any media
 *	change magic here.
 */
int fd_open(uint8_t minor, uint16_t flag)
{
	if (minor >= MAX_FD || present[minor] == 0) {
		udata.u_error = ENODEV;
		return -1;
	}
	/* In case we changed media */
	tracknum[minor] = 255;
	return 0;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	return fd_transfer(minor, true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	return fd_transfer(minor, false, rawflag);
}

/*
 *	A drive that exists will respond to restore and will assert track0
 */
static int fd_probe_drive(int unit)
{
	fd_set_side(unit, 0);
	fd_set_reg(FDC_CS, FDC_RESTORE | FDC_HBIT | step[unit]);
	if (fd_wait() == 0) {
		if (fd_get_reg(FDC_CS) & FDC_TRACK0) {
			dma_unlock();
			/* The Falcon might have HD but we'll deal with that in the far future! */
			kprintf("fd%d: double density.\n", unit);
			present[unit] = 1;
			return 1;
		}
	}
	return 0;
}

void fd_probe(void)
{
	/* Do we need to deal with waiting for motor off here ? */
	dma_lock();
	fd_probe_drive(0);
	fd_probe_drive(1);
	dma_unlock();
}
