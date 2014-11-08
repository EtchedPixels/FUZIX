#include <kernel.h>
#include <printf.h>
#include <version.h>
#include <kdata.h>
#include <devices.h>
#include <devhd.h>
#include <devfd.h>
#include <devlpr.h>
#include <devsys.h>
#include <tty.h>
#include <devtty.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/fd		Floppy disc block devices */
  {  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },
  /* 1: /dev/hd		Hard disc block devices (and RAM etc) */
  {  hd_open,     no_close,    hd_read,   hd_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,     no_close,   no_rdwr,   lpr_write,  no_ioctl  },
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

/*
 *	We may want to do this differently in the end but report the
 *	cartridge hashes found.
 *
 *	A4B6 = Sony HBD-F1
 *	3B49 = Sony HBK-30
 */
void device_init(void)
{
  int i;
  for (i = 0; i < 16; i++) {
    if (slot_table[i])
      kprintf("%d.%d: %x\n", (i>>2) & 3, i & 3, slot_table[i]);
  }
}
