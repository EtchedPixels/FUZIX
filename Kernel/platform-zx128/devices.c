#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsys.h>
#include <vt.h>
#include <devmdv.h>
#include <devide.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/fd		Floppy disc block devices, or microdrive etc */
  {  mdv_open,     mdv_close,    mdv_read,   mdv_write,   no_ioctl },
#ifdef CONFIG_IDE
  /* 1: /dev/hd		Hard disc block devices */
  {  devide_open,  no_close, devide_read,   devide_write,   no_ioctl },
#else
  {  no_open,  no_close, no_read,   no_write,   no_ioctl },
#endif
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

void device_init(void)
{
#ifdef CONFIG_IDE
  devide_init();
#endif
}
