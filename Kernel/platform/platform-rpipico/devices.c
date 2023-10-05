#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>
#include <blkdev.h>
#include <tty.h>
#include <devtty.h>
#include <dev/devsd.h>
#include <printf.h>
#include "globals.h"
#include "picosdk.h"
#include <hardware/irq.h>
#include <hardware/structs/timer.h>
#include <pico/multicore.h>
#include "core1.h"

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write           ioctl
// ---------------------------------------------------------------------
  /* 0: /dev/hd - block device interface */
  {  blkdev_open,   no_close,   blkdev_read,    blkdev_write,	blkdev_ioctl},
  /* 1: /dev/fd - Floppy disk block devices */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,     no_close,   no_rdwr,   no_rdwr,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
};

static absolute_time_t now;

bool validdev(uint16_t dev)
{
    /* This is a bit uglier than needed but the right hand side is
       a constant this way */
    if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) - 1)
	return false;
    else
        return true;
}

static void timer_tick_cb(unsigned alarm)
{
    absolute_time_t next;
    update_us_since_boot(&next, to_us_since_boot(now) + (1000000 / TICKSPERSEC));
    if (hardware_alarm_set_target(0, next)) 
    {
        update_us_since_boot(&next, time_us_64() + (1000000 / TICKSPERSEC));
        hardware_alarm_set_target(0, next);
    }

    timer_interrupt();

    if (usbconsole_is_readable())
    {
        uint8_t c = usbconsole_getc_blocking();
        tty_inproc(minor(BOOT_TTY), c);
    }
}

void device_init(void)
{
    /* The flash device is too small to be useful, and a corrupt flash will
     * cause a crash on startup... oddly. */

	flash_dev_init();
    
	sd_rawinit();
	devsd_init();

    hardware_alarm_claim(0);
    update_us_since_boot(&now, time_us_64());
    hardware_alarm_set_callback(0, timer_tick_cb);
    timer_tick_cb(0);
}

/* vim: sw=4 ts=4 et: */

