#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
#include <devdw.h>
#include <devsys.h>
#include <devlpr.h>
#include <tty.h>
#include <vt.h>
#include <graphics.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <devscsi.h>
#include <devsd.h>
#include <device.h>
#include <carts.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices (SD and IDE) */
  {  blkdev_open,  no_close,	blkdev_read,	blkdev_write,	blkdev_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,    my_tty_close, tty_read,  tty_write,  gfx_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,     lpr_close,   no_rdwr,   lpr_write,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* 8: /dev/dw		DriveWire remote disk images */
  {  dw_open,      no_close,    dw_read,   dw_write,  dw_ioctl },
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

uint16_t ide_base = 0xFF50;
uint8_t ide_slot = 3;		/* Disk in slot 3 by convention */
uint8_t scsi_slot = 3;		/* Again by convention */

static uint8_t old_slot;

void ide_select(uint8_t device)
{
  if (cartslots > 1)
    old_slot = mpi_set_slot(ide_slot);
}

void ide_deselect(void)
{
  if (cartslots > 1)
    mpi_set_slot(old_slot);
}

/* This can move to discard... */

void device_init(void)
{
    int i;

#ifdef CONFIG_SD
    if (spi_setup())
        devsd_init();
#endif
#ifdef CONFIG_IDE
    i = cart_find(CART_IDE);
    if (i >= 0) {
      ide_base = cartaddr[i] ? cartaddr[i]: ide_base;
      ide_slot = i;
      devide_init();
    }
#endif
#ifdef CONFIG_SCSI
    i = cart_find(CART_TC3);
    if (i >= 0) {
      scsi_slot = i;
      devscsi_init();
    }
#endif
}
