#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devices.h>
#include <devfd.h>
#include <devsys.h>
#include <devlpr.h>
#include <devtty.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* 0: /dev/fd		Floppy disc block devices  */
  {  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },
  /* 1: /dev/hd		Hard disc (we use for ROMs) */
  {  rom_open,    no_close,    rom_read,   rom_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
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
    if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) - 1)
	return false;
    else
        return true;
}

__sfr __at 0x19 ioctrlr;

/* FIXME: find correct initial value */
static uint8_t ioctrlr_shadow = 0x00;

/* We need to track the state of the ioctrlr port as it is a shared
   resource (7 = speaker, 6-4 LEDs, 3 - !cartridge reset, 2 serial out ctrl
  1 !printer initial output, 0 lpt strobe */
  
void mod_ioctrlr(uint8_t set, uint8_t clr)
{
  ioctrlr_shadow &= ~clr;
  ioctrlr_shadow |= set;
  ioctrlr = ioctrlr_shadow;
}
