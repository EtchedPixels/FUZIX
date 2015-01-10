#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devtty.h>
#include <devide.h>
#include <devsd.h>
#include <blkdev.h>
#include <ds1302.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
/*   open	    close	read		write		ioctl */
  {  blkdev_open,   no_close,	blkdev_read,	blkdev_write,	blkdev_ioctl },	/* 0: /dev/hd -- standard block device interface */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl },	/* 1: unused slot */
  {  tty_open,	    tty_close,	tty_read,	tty_write,	tty_ioctl },	/* 2: /dev/tty -- serial ports */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl },	/* 3: unused slot */
  {  no_open,	    no_close,	sys_read,	sys_write,	sys_ioctl  },	/* 4: /dev/mem etc	System devices (one offs) */
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
    devsd_init();
    ds1302_init();
}
