#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devdw.h>
#include <devsys.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <tinysd.h>
#include <dwtime.h>
#include <netdev.h>
#include <devlpr.h>
#include <devrtsd.h>
#include <devdw.h>
#include <ttydw.h>
#include <devsdc.h>

struct devsw dev_tab[] =	/* The device driver switch table */
{
	/* 0: /dev/hd         Hard disc block devices */
	{ td_open,	no_close,	td_read,	td_write,	td_ioctl },
	/* 1: /dev/fd         Floppy disc block devices  */
	{ nxio_open,	no_close,	no_rdwr,	no_rdwr,	no_ioctl },
	/* 2: /dev/tty        TTY devices */
	{ tty_open, 	my_tty_close,	tty_read,	tty_write,	gfx_ioctl },
	/* 3: /dev/lpr        Printer devices */
	{ lpr_open,	lpr_close,	no_rdwr,        lpr_write,	no_ioctl },
	/* 4: /dev/mem etc    System devices (one offs) */
	{ no_open,	sys_close,	sys_read,       sys_write,	sys_ioctl },
	/* Pack to 7 with nxio if adding private devices and start at 8 */
	{ nxio_open,	no_close,	no_rdwr,        no_rdwr,	no_ioctl },
	{ nxio_open,	no_close,	no_rdwr,        no_rdwr,	no_ioctl },
	{ nxio_open,	no_close,	no_rdwr,        no_rdwr,	no_ioctl },
	/* /dev/dw   Drivewire */
	{ dw_open,     no_close,	dw_read,	dw_write,	dw_ioctl },
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

void device_init(void)
{
#ifdef CONFIG_COCOSDFPGA
	sd_probe();
#endif
#ifdef CONFIG_COCOIDE
	ide_probe();
#endif
#ifdef CONFIG_COCOSDC
	devsdc_probe();
#endif
#ifdef CONFIG_COCOSDNANO
	devrtsd_probe();
#endif
	dw_init();
	inittod();
#ifdef CONFIG_NET
	sock_init();
#endif
}
