#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devhd.h>
#include <devfd.h>
#include <devlpr.h>
#include <devsys.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <devsd.h>
#include <blkdev.h>
#include "msx2.h"
#include <printf.h>

extern int megasd_probe();

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/fd		Floppy disc block devices */
  {  no_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* 1: /dev/hd		MegaSD Interface */
  {  blkdev_open,    no_close,   blkdev_read,  blkdev_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  vt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,     no_close,   no_rdwr,   no_rdwr,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read, sys_write, sys_ioctl  },
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

DISCARDABLE

void device_init(void)
{
#ifdef CONFIG_RTC
    inittod();
#endif

    kprintf ("Running on a ");
    if (machine_type == MACHINE_MSX1) {
	kprintf("MSX1 not supported\n");
	// hang!
    } else if (machine_type == MACHINE_MSX2) {
	kprintf("MSX2 ");
    } else if (machine_type == MACHINE_MSX2P) {
        kprintf("MSX2+ ");
    } else if (machine_type == MACHINE_MSXTR) {
	kprintf("MSX TurboR ");
    }

    if ((infobits & KBDTYPE_MASK) == KBDTYPE_JPN) {
	kprintf("JP ");
    } else {
	kprintf("INT ");
    }
    if ((infobits & INTFREQ_MASK) == INTFREQ_60Hz) {
	kprintf("60Hz\n");
	ticks_per_dsecond = 6;
    } else {
	kprintf("50Hz\n");
	ticks_per_dsecond = 5;
    }

    if (megasd_probe()) {
        /* probe for megaflash rom sd */
        devsd_init();
    }
}
