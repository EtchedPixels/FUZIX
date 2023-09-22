#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <devfd.h>
#include <devrd.h>
#include <devtty.h>
#include <tinydisk.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
/*   open	    close	read		write		ioctl */
  /* 0: /dev/hd - block device interface */
  {  td_open,	    no_close,   td_read,	td_write,	td_ioctl},
  /* 1: /dev/fd - Floppy disk block devices */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
  /* 2: /dev/tty -- serial ports */
  {  tty_open,      tty_close,	tty_read,	tty_write,	tty_ioctl},
  /* 3: RAM disk */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
  /* 4: /dev/mem etc      System devices (one offs) */
  {  no_open,	    no_close,	sys_read,	sys_write,	sys_ioctl},
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
