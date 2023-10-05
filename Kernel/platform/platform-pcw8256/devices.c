#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
#include <tinydisk.h>
#include <devsys.h>
#include <devlpr.h>
#include <devtty.h>
#include <tty.h>
#include <vt.h>
#include <pcw8256.h>
#include <devfdc765.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices (UIDE or FIDHD) */
  {  td_open, 	no_close,	td_read,     td_write,     td_ioctl	},
  /* 1: /dev/fd		Floppy disc block devices */
  {  devfd_open,  no_close,     devfd_read,  devfd_write,  no_ioctl     },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,    pcwtty_close, tty_read,    tty_write,    vt_ioctl     },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,    lpr_close,    no_rdwr,     lpr_write,    no_ioctl     },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,     no_close,     sys_read,    sys_write,    sys_ioctl    },
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
