#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsd.h>
#include <devsys.h>
#include <blkdev.h>
#include <printf.h>
#include <timer.h>
#include "msp430fr5969.h"
#include "externs.h"

extern uint8_t last_interrupt;

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/sd		SD disk  */
  {  blkdev_open,   no_close,    blkdev_read, blkdev_write, blkdev_ioctl },
  /* 1: /dev/hd		Hard disc block devices (sd card) */
  {  nxio_open,     no_close,    no_rdwr,     no_rdwr,      no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,      tty_close,   tty_read,    tty_write,    tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,      no_close,     no_rdwr,     no_rdwr,      no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,     sys_read,    sys_write,    sys_ioctl  },
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

/* This is called with interrupts off. */
void plt_interrupt(void)
{
	switch (last_interrupt)
	{
		case INTERRUPT_WDT:
			timer_interrupt();
			break;

		case INTERRUPT_USCI_A0:
			tty_interrupt();
			break;
	}
}


