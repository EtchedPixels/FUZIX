#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
#include <devdw.h>
#include <devsys.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <mini_ide.h>
#include <device.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices (IDE) */
  {  ide_open,	    no_close,	 ide_read,  ide_write, ide_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  no_open,       no_close,    no_rdwr,   no_rdwr ,  no_ioctl },
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
    ide_probe();
}
