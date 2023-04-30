#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <devsd.h>
#include <ds1302.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/hd		Hard disc block devices (IDE) */
  {  blkdev_open,   no_close,	blkdev_read,	blkdev_write,	blkdev_ioctl },
  /* 1: /dev/fd		Floppy disc block devices  */
  {  no_open,       no_close,   no_rdwr,   no_rdwr,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,      tty_close,  tty_read,  tty_write, tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,       no_close,   no_rdwr,   no_rdwr,   no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,       no_close,   sys_read, sys_write,  sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
  {  nxio_open,     no_close,   no_rdwr,   no_rdwr,   no_ioctl },
  {  nxio_open,     no_close,   no_rdwr,   no_rdwr,   no_ioctl },
  {  nxio_open,     no_close,   no_rdwr,   no_rdwr,   no_ioctl },
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

/* This can move to discard... */

struct piaregs
{
  uint8_t pra;
  uint8_t ctrla;
  uint8_t prb;
  uint8_t ctrlb;
};

#define pia	((volatile struct piaregs *)0xFE68)

void device_init(void)
{
    pia->ctrla |= 4;
    pia->pra = 0xFE;	/* lines high/low when we set direction */
    pia->ctrla &= ~4;
    pia->pra = 0x07;	/* cs, data out, clock as output, data in as input */
    pia->ctrla |= 4;

    ds1302_init();
#ifdef CONFIG_IDE
    devide_init();
#endif
#ifdef CONFIG_SD
    devsd_init();
#endif
}
