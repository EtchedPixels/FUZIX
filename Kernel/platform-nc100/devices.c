#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devrd.h>
#include <devsys.h>
#include <devlpr.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/fd		Floppy disc block devices (NC200 only) */
  {  nxio_open,     no_close,    no_rdwr,   no_rdwr,   no_ioctl },
  /* 1: /dev/hd		Hard disc block devices (Really PCMCIA) */
  {  rd_open,     no_close,    rd_read,   rd_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  nc100_tty_open,     nc100_tty_close,   tty_read,  tty_write,  vt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,     lpr_close,   no_rdwr,   lpr_write,  no_ioctl  },
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

void device_init(void)
{
  inittod();
  nc100_tty_init();
}

__sfr __at 0x30 control;
static uint8_t control_shadow = 0x10;

/* We need to track the state of the control port */
void mod_control(uint8_t set, uint8_t clr)
{
  control_shadow &= ~clr;
  control_shadow |= set;
  control = control_shadow;
}
