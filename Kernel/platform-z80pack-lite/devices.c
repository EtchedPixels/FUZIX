#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
#include <devmem.h>
#include <devzero.h>
#include <devnull.h>
#include <devproc.h>
#include <devlpr.h>
#include <devtty.h>


struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write       ioctl
// -----------------------------------------------------------------
  /* Floppy disc block devices  */
  {  0,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   0   /dev/fd0
  {  1,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   1   /dev/fd1
  {  2,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   2   /dev/rd2
  {  3,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   3   /dev/rd3
  /* Hard disc block devices */
  {  8,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   4   /dev/sd0 
  {  9,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   5   /dev/sd1 
  {  10, fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   6   /dev/sd2 
  /* Devices below here are not mountable (as per NDEVS) */
  {  0, lpr_open,     lpr_close,   no_rdwr,   lpr_write,  no_ioctl  },  //  7   /dev/lp  
  {  0, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  8   /dev/tty
  {  1, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  9   /dev/tty1
  {  2, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  10   /dev/tty2
  {  3, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  11   /dev/tty3
  {  4, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  12   /dev/tty4
  {  0, no_open,      no_close,    null_read, null_write, no_ioctl  },  //  13   /dev/null
  {  0, no_open,      no_close,    zero_read, no_rdwr,    no_ioctl  },  //  14   /dev/zero
  {  0, no_open,      no_close,    mem_read,  mem_write,  no_ioctl  },  //  15   /dev/kmem
  {  0, no_open,      no_close,    proc_read, no_rdwr, proc_ioctl}      //  16   /dev/proc
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
