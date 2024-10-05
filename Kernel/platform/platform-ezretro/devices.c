#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devfd.h>
#include <devsys.h>
#include <devtty.h>
#include <devrtc.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices */
  {  hd_open,     no_close,    hd_read,   hd_write,   no_ioctl  },
  /* 1: /dev/fd		Floppy disc block devices */
  {  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl  },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,    tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  rd_open,      no_close,   rd_read,   rd_write,   no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      sys_close,  sys_read,  sys_write,  sys_ioctl },
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

void device_init(void)
{
}
