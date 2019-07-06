/*
 *	Fuzix SCSI/SASI inteface logic
 *
 *	Based upon the code from OMU68K by the late Dr Steve Hosgood &
 *	Terry Barnaby, licensed under the GPLv2.
 */

#define _SCSI_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devscsi.h>
#include <blkdev.h>

#ifdef CONFIG_SCSI

/* The live command block */
struct Sidcmd si_dcb;
uint8_t si_user;

/* If we get more then union this lot */
static struct Sierr error;
static uint8_t status[2];


/*
 *	Si_cmdend()	Wait till end of scsi command and return errors.
 */
static uint8_t si_cmdend(void) {
	int err;

	status[0] = 0xFF;
	si_dcb.length = 2;
	si_user = 0;
	/* Gets status and message byte */
	if((err = si_read(status)) != 0)
		return err;
	else return status[0];
}

/*
 *	Si_err()	SCSI error recovery
 */
static int si_err(const char *str, int err)
{
	kprintf("scsi%d: %s dev %d cmd %x err %d\n",
		si_dcb.bus, str, si_dcb.device, si_dcb.cb.cb0.opcode, err);

	/* Get error info from drive */
	si_dcb.cb.cb0.opcode = SIREQSENSE;
	si_dcb.cb.cb0.hiblock = 0;
	si_dcb.cb.cb0.miblock = 0;
	si_dcb.cb.cb0.loblock = 0;
	si_dcb.cb.cb0.noblocks = 18;
	si_dcb.cb.cb0.control = 0;
	si_dcb.length = 18;
	si_dcb.direction = SIDIR_READ;
	si_user = 0;

	if (si_docmd((uint8_t *)&error)) {
		kprintf("sd: no sense\n", si_dcb.device);
		return -1;
	}
	kprintf("sd: sense %x err %x\n",error.sensekey,
		error.errcode);
	return 0;
}

/*
 *	Si_docmd()	Do si cmd
 */

int si_docmd(uint8_t *data)
{
	int	err;

	/* Select the drive */
	if((err = si_select()) != 0)
		return err;

	/* Send the command block */
	if(SICLASS(si_dcb.cb.cb0.opcode) == 1)
		err = si_writecmd(sizeof(struct Sidcb1));
	else
		err = si_writecmd(sizeof(struct Sidcb0));

	if (err)
		return err;

	/* Do command actions */
	if (si_dcb.length) {
		if (si_dcb.direction == SIDIR_READ)	/* data in */
			err = si_read(data);
		else
			err = si_write(data);
	}
	if (err == 0)
		err = si_cmdend();
	si_clear();		/* Clears any data on scsi bus */
	si_deselect();
	return 0;
}

/*
 *	Si_cmd()	Execute SCSI command with given data
 *			Perform error recovery (retry command)
 */

uint8_t si_cmd(void)
{
	int	count, err;

	/* FIXME: we need some kind of mapping helper from the platform */
	si_dcb.device = blk_op.blkdev->driver_data & DRIVE_NR_MASK;
	si_dcb.lun = 0;
	si_dcb.bus = 0;
	
	/* Very big disks might need READ10/WRITE10 etc, however if we add
	   them we need to use them only on big devices as they are not in
	   early devices or SASI */

	/* Retry command */
	for(count = 0; count < 10; count++) {
		/* Sets up command device control block */
		si_dcb.direction = blk_op.is_read ? SIDIR_READ : SIDIR_WRITE;
		si_dcb.cb.cb0.opcode = blk_op.is_read ? SIREAD: SIWRITE;
		si_dcb.cb.cb0.hiblock = (blk_op.lba >> 16) & 0x1F;
		si_dcb.cb.cb0.miblock = (blk_op.lba >> 8) & 0xFF;
		si_dcb.cb.cb0.loblock = blk_op.lba & 0xFF;
		/* FIXME: if we do non 512 byte blocks we need to be smarter here */
		si_dcb.cb.cb0.noblocks = blk_op.nblock;
		si_dcb.cb.cb0.control = 0;
		si_dcb.length = 512 * blk_op.nblock;

		/* User or kernel target ? */
		si_user = blk_op.is_user;

		if(!(err = si_docmd(blk_op.addr)))
		  break;
	}

	/* If any errors print error message */
	if(count) {
		/* Caution: si_err trashes si_dcb */
		/* If fatal error */
		if(count == 10){
			si_err("Fatal error\007 10 retries", err);
                        /* Ugly - si_reset is odd and has to do its own
                           select/deselect of the controller if needed */
			si_reset();
			return 0;
		}
		/* If recoverable error */
		else {
			si_err("Recovered",  err);
		}
	}
	return blk_op.nblock;
}

/* This needs plumbing into a /dev/sg device off devsys */

int si_ioctl(uint_fast8_t dev, uarg_t req, char *data)
{
	struct	Siioctl sip;

	/* Upper layer already checked for root */
	if (req != HDIO_RAWCMD)
		return -1;

	/* By a cunning co-incidence the kernel and user ioctl block happen
	   to look identical */
	if (uget(data, &sip, sizeof(sip)))
		return -1;
	memcpy(&si_dcb, &sip.si_dcb, sizeof(si_dcb));

	/* Make sure the address given for the user mode I/O is valid */
	if (!valaddr((void *)sip.si_data, sip.si_dcb.length))
		return -1;
	/* Set it up as a user mode read/write */
	blk_op.addr = sip.si_data;
	blk_op.is_user = 1;
	/* Issue the command block */
	return si_docmd(sip.si_data);
}

/* TODO: issue a cache flush */
int si_flush(void)
{
        return 0;
}

#endif /* CONFIG_SCSI */
