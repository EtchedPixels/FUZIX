#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <vt.h>
#include <devmdv.h>
#include <devfd.h>
#include <betadisk.h>
#include <devide.h>
#include <blkdev.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
#ifdef CONFIG_BETADISK
  /* 0: /dev/fd Floppy disc block devices: betadisk */
  {  betadisk_open, no_close, betadisk_read,  betadisk_write, no_ioctl },
#else
  /* 0: /dev/fd	Floppy disc block devices: disciple */
  {  fd_open,      no_close,     fd_read,  fd_write,   no_ioctl },
#endif
#ifdef CONFIG_IDE
  /* 1: /dev/hd		Hard disc block devices */
  {  blkdev_open,  no_close,     blkdev_read,   blkdev_write,  blkdev_ioctl },
#else
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl },
#endif
  /* 2: /dev/tty	TTY devices */
  {  tty_open,	   tty_close,    tty_read,      tty_write,     vt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,     sys_read,      sys_write,     sys_ioctl  },
  /* 5: Pack to 7 with nxio if adding private devices and start at 8 */
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 8: /dev/mdv		Microdrive */
  {  mdv_open,     mdv_close,    mdv_read,   mdv_write,   no_ioctl },
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

#ifdef SWAPDEV
  /* Hack for now - we need to open the swap to get the map. Should
     we open swap nicely somewhere generic ? */
  d_open(SWAPDEV, 0);
#endif
}
