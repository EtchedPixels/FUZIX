#include <kernel.h>
#include <tty.h>
#include <version.h>
#include <kdata.h>
#include <devrd.h>
#include <devsys.h>
#include <devlpr.h>
#include <devtty.h>
#include <devsd.h>
#include <blkdev.h>
#include <deveth.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/sd		SD disc block devices  */
  {  blkdev_open,  no_close,	blkdev_read,	blkdev_write,	blkdev_ioctl },
  /* 1: /dev/hd		Hard disc block devices (RAMdisc) */
  {  rd_open,      no_close,    rd_read,   rd_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,     lpr_close,   no_rdwr,   lpr_write,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read,  sys_write,  sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
  {  no_open,      no_close,    sys_read,  sys_write,  sys_ioctl  },
  {  no_open,      no_close,    sys_read,  sys_write,  sys_ioctl  },
  {  no_open,      no_close,    sys_read,  sys_write,  sys_ioctl  },
  /* 8: Ethernet */
  {  eth_open,     eth_close,   eth_read,  eth_write,  eth_ioctl  },
};

bool validdev(uint16_t dev)
{
    /* This is a bit uglier than needed but the right hand side is
       a constant this way */
    if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) - 1)
	return false;
    else
        return true;
}

void device_init(void)
{
  devsd_init();
  deveth_init();
}
