#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
#include <devsys.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <device.h>
#include <tinydisk.h>
#include <sd.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices (IDE) */
  {  td_open,	    no_close,	 td_read,  td_write, no_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  fd_open,       no_close,    fd_read,  fd_write,  no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,      tty_close,   tty_read,  tty_write, vt_ioctl },
    /* 3: /dev/lpr	Printer devices */
  {  no_open,       no_close,    no_rdwr,   no_rdwr,   no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,       no_close,    sys_read, sys_write,  sys_ioctl  },
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
  to_sd_probe();
}
