#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <vt.h>
#include <devide.h>
#include <devsd.h>
#include <blkdev.h>
#include <devtty.h>
#include <devfdc765.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/hd		Hard disc block devices */
  {  blkdev_open,  no_close,     blkdev_read,   blkdev_write,  blkdev_ioctl },
  /* 1: /dev/fd		Floppy disc block devices */
  {  devfd_open, no_close, devfd_read, devfd_write, no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,	   tty_close,    tty_read,      tty_write,     vt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,     sys_read,      sys_write,     sys_ioctl  },
  /* 5: Pack to 7 with nxio if adding private devices and start at 8 */
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
#ifdef CONFIG_IDE
  devide_init();
#endif
#ifdef CONFIG_SD
  devsd_init();
#endif
}
