#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devtty.h>
#include <devide.h>
#include <blkdev.h>
#include <ds1302.h>
#include <devrd.h>
#ifdef CONFIG_P112_FLOPPY
#include <devfd.h>
#endif

struct devsw dev_tab[] =  /* The device driver switch table */
{
/*   open	    close	read		write		ioctl */
  {  blkdev_open,   no_close,	blkdev_read,	blkdev_write,	blkdev_ioctl },	/* 0: /dev/hd -- standard block device interface */
#ifdef CONFIG_P112_FLOPPY
  {  fd_open,	    fd_close,	fd_read,	fd_write,	no_ioctl },	/* 1: /dev/fd -- floppy drive */
#else
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl },	/* 1: unused slot */
#endif
  {  tty_open,	    tty_close,	tty_read,	tty_write,	tty_ioctl },	/* 2: /dev/tty -- serial ports */
  {  rd_open,	    no_close,	rd_read,	rd_write,	no_ioctl },	/* 3: /dev/rd?, /dev/physmem -- memory backed devices */
  {  no_open,	    no_close,	sys_read,	sys_write,	sys_ioctl  },	/* 4: /dev/mem etc	System devices (one offs) */
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
