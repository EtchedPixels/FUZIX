#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devgm833.h>
#include <nascom.h>

/*
 *	GM833 I/O mapped RAM disc
 *
 *	512K of RAM on I/O ports. A maximum of 16 boards can in theory be used
 *	although the reality is probably a limit of 4, and that was all CP/M
 *	coped with. It's organized as an array of 128 byte sectors, with 256
 *	per track. In other words its a 16bit block address, 128 byte sector
 *	device. That makes it really simple to drive.
 *
 *	Multiple cards form one bigger ramdisc, with a theoretical 8MB max but
 *	we expose each card as its own minor device as well as a single unified
 *	device so you can pick. Just don't mix!
 *
 *	Use minor 0 for 'all', minors 1-16 for units.
 */

__sfr __at 0xfb gm833_track;
__sfr __at 0xfc gm833_sector;

static uint8_t openshift;
static uint8_t openmask;
static uint8_t num_gm833;

static int gm833_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	int ct = 0;
	uint8_t err = 0;

	/* We support swap, it's after all an ideal swap device! */
	io_page = 0;
	if (rawflag == 1) {
		if (d_blkoff(BLKSHIFT))
			return -1;
		io_page = udata.u_page;
	} else if (rawflag == 2)
		io_page = swappage;

	/* Shift later minors by 512K per unit */
	if (minor) {
		if (udata.u_block + udata.u_nblock > 1024) {
			udata.u_error = EIO;
			return -1;
		}
		udata.u_block += 1024 * (minor + openshift - 1);
	} else 
		udata.u_block += 1024 * openshift;
	/* We might overflow but the largest possible configuration is
	   8MB so we always just fit */
	udata.u_nblock *= 4;
	udata.u_block *= 4;

	while (ct < udata.u_nblock) {
		gm833_sector = udata.u_block;
		gm833_track = udata.u_block >> 8;
		if (is_read)
			gm833_in(udata.u_dptr);
		else
			gm833_out(udata.u_dptr);
		udata.u_dptr += 128;
		udata.u_block++;
		ct++;
	}
	return udata.u_nblock << 7;
}

int gm833_open(uint8_t minor, uint16_t flag)
{
	flag;
	if (minor >= num_gm833 - openshift) {
		udata.u_error = ENODEV;
		return -1;
	}
	if (minor == 0) {
		if (openmask) {
			udata.u_error = EBUSY;
			return -1;
		}
	} else
		openmask |= (1 << minor);
	return 0;
}

int gm833_close(uint8_t minor)
{
	openmask &= ~(1 << minor);
	return 0;
}

int gm833_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return gm833_transfer(minor, true, rawflag);
}

int gm833_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return gm833_transfer(minor, false, rawflag);
}

/* Move to discard ?? */

void gm833_init(void)
{
	uint8_t *tmp = tmpbuf();
	int i;

	gm833_sector = 0;
	io_page = 0;

	/* Holes are not allowed according to the config rules. If you have
	   holes it breaks - tough */
	for (i = 0;i < 16; i++) {
		gm833_track = i << 4;
		*tmp = 0x90 | i;
		gm833_out(tmp);
		gm833_in(tmp);
		if (*tmp != (0x90 | i))
			break;
	}
	tmpfree(tmp);

	/* i is now the number of units present */
	num_gm833 = i;
	if (i == 0)
		return;
	/* Steal 512K or 1MB for swap if there is no swap device
	   allocated on disk */
	if (swap_dev == 0) {
		/* No swap was found so bag ram disc 1 */
		swap_dev = 0x0801;
		/* Add swap */
		openshift = i >= 2 ? 2 : 1;
		for (i = 0; i < openshift * 8 ; i++)
			swapmap_init(i);
	}
	kprintf("gm833: %dK found", num_gm833 * 512);
	kprintf(", %dK allocated for swap.\n", openshift * 512);
	kputs("\n");
}
