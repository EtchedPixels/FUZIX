#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devfd.h>
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
  /* Floppy disk block devices  */
  {  0,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   0   /dev/fd0
  {  1,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   1   /dev/fd1
  {  2,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   2   /dev/rd2
  {  3,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   3   /dev/rd3

  /* Disk devices (not all used) */
  {  0,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   4   /dev/sd0 
  {  1,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   5   /dev/sd1 
  {  2,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   6   /dev/sd2 
  {  3,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   7   /dev/sd3 
  {  4,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   8   /dev/sd4 
  {  5,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //   9   /dev/sd5 
  {  6,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //  10   /dev/sd6 
  {  7,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //  11   /dev/sd7 
  {  8,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //  12   /dev/sd8 
  {  9,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //  13   /dev/sd9 
  { 10,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //  14   /dev/sd10
  { 11,  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },   //  15   /dev/sd11

  /* devices below here are not mountable (as per NDEVS) */
  {  0, lpr_open,     lpr_close,   no_rdwr,   lpr_write,  no_ioctl  },  //  16   /dev/lp  
  {  0, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  17   /dev/tty
  {  1, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  18   /dev/tty1
  {  2, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  19   /dev/tty2
  {  3, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  20   /dev/tty3
  {  4, tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },  //  21   /dev/tty4
  {  0, no_open,      no_close,    null_read, null_write, no_ioctl  },  //  22   /dev/null
  {  0, no_open,      no_close,    zero_read, no_rdwr,    no_ioctl  },  //  23   /dev/zero
  {  0, no_open,      no_close,    mem_read,  mem_write,  no_ioctl  },  //  24   /dev/kmem
  {  0, no_open,      no_close,    proc_read, no_rdwr, proc_ioctl}      //  25   /dev/proc
  // {  0,  mt_open,  mt_close, mt_read,  mt_write,   nogood }          // 26 /dev/mt  
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
  tty_init();
  fd_probe();
}
