#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devdw.h>
#include <devsys.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#ifdef CONFIG_IDE
#include <devide.h>
#endif
#ifdef CONFIG_SD
#include <devsd.h>
#endif
#include <blkdev.h>
/* [NAC HACK 2016Apr24] add ds1302 */

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices (absent) */
  {  blkdev_open,   no_close,    blkdev_read,	blkdev_write,	blkdev_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     my_tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,       no_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* /dev/dw   Drivewire */
  {  dw_open,       no_close,    dw_read,   dw_write,  dw_ioctl },
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
	devide_init( );
#endif

#ifdef CONFIG_SD
	devsd_init( );
#endif
}

