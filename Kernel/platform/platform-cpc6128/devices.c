#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <vt.h>
#include <tinydisk.h>
#include <tinyide.h>
/*#include <tinysd.h>*/
#include <devtty.h>
#include <devfdc765.h>
/*#include <netdev.h>*/
#include <devrd.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/hd		Hard disc block devices */
  {  td_open,      no_close,     td_read,	td_write,	td_ioctl },
  /* 1: /dev/fd		Floppy disc block devices */
  {  devfd_open,   no_close,     devfd_read,    devfd_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,	   tty_close,    tty_read,      tty_write,     cpcvt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,     sys_read,      sys_write,     sys_ioctl  },
  /* 5: Pack to 7 with nxio if adding private devices and start at 8 */
  #if defined EXTENDED_RAM_512 || defined EXTENDED_RAM_1024
  /* 5: unused */
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 6: unused */
  {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 7: unused */
   {  no_open,      no_close,     no_rdwr,       no_rdwr,       no_ioctl  },
  /* 8: Standard memory expansions RAM swap */
  {  rd_open,      no_close,     rd_read,       rd_write,      no_ioctl  },
  #endif
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

void device_init(void)
{
#ifdef CONFIG_TD_IDE
  ide_probe();
#endif
#ifdef CONFIG_TD_SD
  sd_probe();
#endif
ch375_probe();

#ifdef CONFIG_NET
  sock_init();
#endif
}

