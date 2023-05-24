#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devfd.h>
#include <devrd.h>
#include <devtty.h>
#include <tinydisk.h>
#include <ds1302.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
/*   open	    close	read		write		ioctl */
  /* 0: /dev/hd - block device interface */
#ifdef CONFIG_TD_NUM
  {  td_open,	    no_close,	td_read,	td_write,	td_ioctl},
#else
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
#endif
  /* 1: /dev/fd - Floppy disk block devices */
#ifdef CONFIG_FLOPPY
  {  fd_open,	    fd_close,	fd_read,	fd_write,	no_ioctl},
#else
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
#endif
  /* 2: /dev/tty -- serial ports */
  {  tty_open,	    tty_close,	tty_read,	tty_write,	tty_ioctl},
  /* 3: RAM disk */
#ifdef CONFIG_RAMDISK
  {  rd_open,	    no_close,	rd_read,	rd_write,	no_ioctl},
#else
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
#endif
  /* 4: /dev/mem etc      System devices (one offs) */
  {  no_open,	    no_close,	sys_read,	sys_write,	sys_ioctl},
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
