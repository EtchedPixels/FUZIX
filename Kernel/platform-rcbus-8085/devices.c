#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>


struct devsw dev_tab[5] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices */
  {  blkdev_open, no_close,    blkdev_read,   blkdev_write, blkdev_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  nxio_open,   no_close,    no_rdwr,       no_rdwr,      no_ioctl     },
  /* 2: /dev/tty	TTY devices */
  {  rctty_open,  tty_close,   tty_read,      tty_write,    rctty_ioctl  },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,     no_close,    no_rdwr,       no_rdwr,      no_ioctl     },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,     sys_close,   sys_read,      sys_write,    sys_ioctl    },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
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
