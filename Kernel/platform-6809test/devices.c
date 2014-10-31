#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devrd.h>
#include <devmem.h>
#include <devzero.h>
#include <devnull.h>
#include <devproc.h>
#include <devlpr.h>
#include <tty.h>
#include <devtty.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* Memory disk block devices  */
  {  0,  rd_open,     no_close,    rd_read,   rd_write,   no_ioctl },   //   0   /dev/rd0

  /* devices below here are not mountable (as per NDEVS) */
  {  0, lpr_open,     lpr_close,   no_rdwr,   lpr_write,  no_ioctl  },  //  1   /dev/lp  
  {  0, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  2   /dev/tty
  {  1, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  3   /dev/tty1
  {  2, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  4   /dev/tty2
  {  0, no_open,      no_close,    null_read, null_write, no_ioctl  },  //  5   /dev/null
  {  0, no_open,      no_close,    zero_read, no_rdwr,    no_ioctl  },  //  6   /dev/zero
  {  0, no_open,      no_close,    mem_read,  mem_write,  no_ioctl  },  //  7   /dev/kmem
  {  0, no_open,      no_close,    proc_read, no_rdwr, proc_ioctl}      //  8  /dev/proc
  /* Add more tty channels here if available, incrementing minor# */
};

bool validdev(uint8_t dev)
{
    if(dev >= (sizeof(dev_tab)/sizeof(struct devsw)))
        return false;
    else
        return true;
}

void device_init(void)
{
}

