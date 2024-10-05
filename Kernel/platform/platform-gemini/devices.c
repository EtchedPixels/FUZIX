#include <kernel.h>
#include <tty.h>
#include <version.h>
#include <kdata.h>
#include <devgm8x9.h>
#include <devsys.h>
#include <vt.h>
#include <devtty.h>
#include <tinydisk.h>
#include <gm833.h>

struct devsw dev_tab[] =	/* The device driver switch table */
{
	/* 0: /dev/hd         SCSI/SASI block devices */
	{td_open, no_close, td_read, td_write, td_ioctl},
	/* 1: /dev/fd         Floppy disc block devices */
	{gm8x9_open, no_close, gm8x9_read, gm8x9_write, no_ioctl},
	/* 2: /dev/tty        TTY devices */
	{tty_open, tty_close, tty_read, tty_write, tty_ioctl},
	/* 3: /dev/lpr        Printer devices */
	{nxio_open, no_close, no_rdwr, no_rdwr, no_ioctl},
	/* 4: /dev/mem etc    System devices (one offs) */
	{no_open, no_close, sys_read, sys_write, sys_ioctl},
	/* Pack to 7 with nxio if adding private devices and start at 8 */
	{nxio_open, no_close, no_rdwr, no_rdwr, no_ioctl},
	{nxio_open, no_close, no_rdwr, no_rdwr, no_ioctl},
	{nxio_open, no_close, no_rdwr, no_rdwr, no_ioctl},
	{gm833_open, no_close, gm833_read, gm833_write, no_ioctl}
};

bool validdev(uint16_t dev)
{
	/* This is a bit uglier than needed but the right hand side is
	   a constant this way */
	if (dev > ((sizeof(dev_tab) / sizeof(struct devsw)) << 8) - 1)
		return false;
	else
		return true;
}
