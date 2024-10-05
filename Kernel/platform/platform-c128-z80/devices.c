#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devtty.h>
#include <vt.h>
#include <blkdev.h>
#include <devrd.h>
#include <devfdc765.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
/*   open	    close	read		write		ioctl */
  /* 0: /dev/hd : block device interface */
#ifdef CONFIG_SD
  {  blkdev_open,   no_close,   blkdev_read,    blkdev_write,	blkdev_ioctl},
#else
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
#endif
#ifdef CONFIG_FLOPPY
  { devfd_open,     no_close,   devfd_read,     devfd_write,    no_ioctl},
#else
  /* 1: /dev/fd : Floppy disk block devices */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
#endif
  /* 2: /dev/tty : serial ports */
  {  tty_open,    tty_close,	tty_read,	tty_write,	vt_ioctl},
  /* 3: RAM disk :  */
  {  rd_open,	    no_close,	rd_read,	rd_write,	no_ioctl},
  /* 4: /dev/mem etc      System devices (one offs) */
  {  no_open,	    no_close,	sys_read,	sys_write,	sys_ioctl},
};

bool validdev(uint16_t dev)
{
	/* This is a bit uglier than needed but the right hand side is
	 a constant this way */
	if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) + 255)
		return false;
	else
		return true;
}
