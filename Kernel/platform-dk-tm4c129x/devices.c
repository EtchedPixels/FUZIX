#include <kernel.h>
#include <kdata.h>
#include <devsys.h>
#include <devsd.h>
#include <blkdev.h>
#include <tty.h>
#ifdef CONFIG_NET
#include <netdev.h>
#include "eth.h"
#endif

struct devsw dev_tab[] =  /* The device driver switch table */
{
//          open       close    read            write           ioctl
// ---------------------------------------------------------------------
  /* 0: /dev/hd - block device interface */
  {  blkdev_open,   no_close,   blkdev_read,    blkdev_write,   blkdev_ioctl },
  /* 1: /dev/fd - Floppy disk block devices */
  {  no_open,       no_close,   no_rdwr,        no_rdwr,        no_ioctl },
  /* 2: /dev/tty - TTY devices */
  {  tty_open,     tty_close,   tty_read,       tty_write,      tty_ioctl },
  /* 3: /dev/lpr - Printer devices */
  {  no_open,       no_close,   no_rdwr,        no_rdwr,        no_ioctl  },
  /* 4: /dev/mem etc - System devices (one offs) */
  {  no_open,       no_close,   sys_read,       sys_write,      sys_ioctl },
#ifdef CONFIG_NET
  /* Pack to 7 with nxio if adding private devices and start at 8 */
  {  nxio_open,     no_close,   no_rdwr,        no_rdwr,        no_ioctl },
  {  nxio_open,     no_close,   no_rdwr,        no_rdwr,        no_ioctl },
  {  nxio_open,     no_close,   no_rdwr,        no_rdwr,        no_ioctl },
  /* 8: /dev/eth ethernet device raw access */
  {  eth_open,      eth_close,  eth_read,       eth_write,      eth_ioctl },
#endif
};

bool validdev(uint16_t dev)
{
  if (dev > ((((sizeof dev_tab) / sizeof(struct devsw)) << 8U) - 1U))
    return false;
  else
    return true;
}

void device_init(void)
{
#ifdef CONFIG_SD
  devsd_init();
#endif
#ifdef CONFIG_NET
#ifndef CONFIG_EK	/* For now TOOD */
  ethdev_init();
#endif
  sock_init();
#endif
}
