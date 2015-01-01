#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devtty.h>
#include <devide.h>
#include <ds1302.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		IDE-8 interface */
  {  devide_open, no_close, devide_read, devide_write, no_ioctl },
  /* 1: /dev/sd		SD interface */
  {  no_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* 2: /dev/tty	serial ports */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Unused slot (pad to keep system devices at index 4) */
  {  no_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
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

void device_init(void)
{
    devide_init();
    ds1302_init();
}
