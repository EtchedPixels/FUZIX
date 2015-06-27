#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsd.h>
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
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,    no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,      tty_close,   tty_read,  tty_write,  tty_ioctl },
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
	/* Configure the watchdog timer to use ACLK as the system interrupt.
	 * ACLK was set up in the boot sequence to use the LFXT clock, which runs
	 * (relatively accurately) at 32kHz. 512 ticks at 32kHz is 64Hz.
	 */

	WDTCTL = WDTPW | WDTSSEL__ACLK | WDTTMSEL | WDTCNTCL | WDTIS__512;
	SFRIE1 |= WDTIE;

	sd_rawinit();
	devsd_init();
}

/* This is called with interrupts off. */
void platform_interrupt(void)
{
	switch (last_interrupt)
	{
		case INTERRUPT_WDT:
			timer_interrupt();
			break;
	}
}


