#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
#include <devsys.h>
#include <blkdev.h>
#include <devlpr.h>
#include <tty.h>
#include <vt.h>
#include <machine.h>
#include <devide.h>
#include <acsi.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices */
  {  blkdev_open, no_close,     blkdev_read, blkdev_write, blkdev_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  fd_open,     no_close,     fd_read,     fd_write,     no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,    tty_close,    tty_read,    tty_write,    vt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,    lpr_close,    no_rdwr,     lpr_write,    no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,     no_close,     sys_read,    sys_write,    sys_ioctl  },
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
	fd_probe();
	if (features & FEATURE_IDE)
		devide_init();
	acsi_init();
}

/* Very simple routines because we have a flat memory space and MMIO */

void devide_read_data(void)
{
	uint16_t ct = 256;
	uint16_t *p = (uint16_t *)blk_op.addr;
	while(ct--)
		*p++ = *ide_data16;
}

void devide_write_data(void)
{
	uint16_t ct = 256;
	uint16_t *p = (uint16_t *)blk_op.addr;
	while(ct--)
		*ide_data16 = *p++;
}